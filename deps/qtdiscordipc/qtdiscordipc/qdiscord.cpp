#include "qdiscord.h"

#include <QJsonDocument>
#include <QRandomGenerator64>
#include <QFile>
#include <QHttpMultiPart>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QUrlQuery>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QSaveFile>
#include <QDir>
#include <QCoreApplication>
#include <QTimer>

struct MessageHeader {
    uint32_t opcode;
    uint32_t length;
};
static_assert(sizeof(MessageHeader) == 8);

double QDiscord::ipcToUIVolume(double v) {
    if(v <= 0)
        return 0;
    else if(v <= 100)
        return 17.362 * log(v) + 20.054;
    else
        return 144.86 * log(v) - 567.21;
}

double QDiscord::uiToIPCVolume(double v) {
    if(v <= 0)
        return 0;
    else if(v <= 100)
        return exp((v - 20.054) / 17.362);
    else
        return exp((v + 567.21) / 144.86);
}

QDiscord::QDiscord() {
    QObject::connect(&socket_, &QLocalSocket::errorOccurred, this, [this](const QLocalSocket::LocalSocketError &err) {
        qWarning() << "QDiscord socket error: " << static_cast<int>(err);
    });
    QObject::connect(&socket_, &QLocalSocket::disconnected, this, [this] {
        qDebug() << "Disconnected";
        if(connectionError_.isEmpty())
            connectionError_ = "DISCONNECTED";
        disconnect();
    });
    QObject::connect(&socket_, &QLocalSocket::readyRead, this, [this] {
        if(blockingRead_)
            return;

        readAndProcessMessages();
        if(socket_.bytesAvailable())
            qDebug() << "ASSERTION FAILED: bytes still available after read and process messages";
    });
}

QDiscord::~QDiscord() {
    for(const auto r: pendingReplies_)
        delete r;
}

bool QDiscord::connect(const QString &clientID, const QString &clientSecret) {
    // connect() processes events while waiting (so the Stream Deck stays responsive),
    // which means it can get called again from a timer or a button press - ignore those calls
    if(connecting_)
        return false;

    connecting_ = true;
    connectionError_ = "CONNECTING";
    processing_++;
    const bool r = [&]() {
        if(clientID.isEmpty() || clientSecret.isEmpty()) {
            qDebug() << "Missing client ID or secret";
            connectionError_ = "ERR 0";
            return false;
        }

        // start connecting
        for(int i = 0; i < 10; i++) {
            socket_.abort();
            socket_.connectToServer(ipcPipeName(i));
            qDebug() << "Trying to connect to Discord (" << i << ")";

            if(socket_.waitForConnected(3000))
                break;
        }
        if(socket_.state() != QLocalSocket::ConnectedState) {
            qDebug() << "Connection failed";
            connectionError_ = "ERR 1";
            return false;
        }

        qDebug() << "Connected";
        static const QStringList scopes{"rpc", "identify"};

        // Handshake and dispatch Receive DISPATCH
        {
            sendMessage(QJsonObject{
                                    {"v",         1},
                                    {"client_id", clientID},
                                    }, 0);

            const QDiscordMessage msg = readMessage(handshakeTimeout);

            if(msg.json.isEmpty()) {
                qWarning() << "QDiscord - empty response" << msg.json;
                connectionError_ = "ERR 8";
                return false;
            }


            if(msg.json["cmd"] != "DISPATCH") {
                qWarning() << "QDiscord - unexpected message (expected DISPATCH)" << msg.json["cmd"];
                connectionError_ = "ERR 2";
                return false;
            }

            cdn_ = msg.data["config"]["cdn_host"].toString();
        }

        QJsonObject oauthData;
        {
            QFile oauthFile(oauthFilePath());
            if(oauthFile.open(QIODevice::ReadOnly))
                oauthData = QJsonDocument::fromJson(oauthFile.readAll()).object();
        }

        // Write to a temporary file and rename, so a crash/kill mid-write can't destroy the stored token
        const auto saveOauthData = [&] {
            QSaveFile f(oauthFilePath());
            if(!f.open(QIODevice::WriteOnly) || f.write(QJsonDocument(oauthData).toJson(QJsonDocument::Compact)) < 0 || !f.commit())
                qWarning() << "QDiscord - failed to save oauth data";
        };

        const auto loadIdentityFromAuth = [&](const QJsonObject &msg) {
            userID_ = msg["data"]["user"]["id"].toString();
        };

        const auto authenticate = [&](const QString &nonce) {
            sendMessage(QJsonObject{
                                    {"cmd",   +CommandType::authenticate},
                                    {"nonce", nonce},
                                    {"args",  QJsonObject{
                                                 {"access_token", oauthData["access_token"].toString()}
                                             }},
                                    });

            const QDiscordMessage msg = readMessage(handshakeTimeout);
            if(msg.json["cmd"] == "AUTHENTICATE" && msg.json["evt"] != "ERROR") {
                loadIdentityFromAuth(msg.json);
                return true;
            }

            qWarning() << "AUTHENTICATE ERROR" << msg.json;
            return false;
        };

        // Authenticate from stored token
        if(!oauthData["access_token"].toString().isEmpty()) {
            if(authenticate("auth_0")) {
                qDebug() << "Connected through pre-stored token";
                return true;
            }

            if(socket_.state() != QLocalSocket::ConnectedState) {
                connectionError_ = "ERR 1";
                return false;
            }
        }

        // Stored access token expired -> try refreshing it.
        // Only done when needed, as Discord rotates refresh tokens on every refresh.
        if(!oauthData["refresh_token"].toString().isEmpty()) {
            QJsonObject newOauthData;
            const QNetworkReply::NetworkError err = requestToken({
                                                                     {"client_id",     clientID},
                                                                     {"client_secret", clientSecret},
                                                                     {"refresh_token", oauthData["refresh_token"].toString()},
                                                                     {"scope",         scopes.join(' ')},
                                                                     {"grant_type",    "refresh_token"},
                                                                 }, newOauthData);

            if(err == QNetworkReply::NoError && !newOauthData["access_token"].toString().isEmpty()) {
                qDebug() << "Successfully refreshed token";
                oauthData = newOauthData;
                saveOauthData();

                if(authenticate("auth_r")) {
                    qDebug() << "Connected through refreshed token";
                    return true;
                }
            }
            else if(err != QNetworkReply::NoError && err < QNetworkReply::ContentAccessDenied) {
                // Connection-level problem (no internet yet, timeout, ...) - the token is probably still fine,
                // so don't throw it away and don't bother the user with the authorization dialog; retry later.
                connectionError_ = "ERR 3";
                return false;
            }
        }

        // When we got here, it mens that the automatic authentication on background failed -> start from scratch
        oauthData = {};

        // Send authorization request
        {
            // Authorize in Discord
            QString authCode;
            {
                connectionError_ = "AUTHORIZE";
                emit connectionStateChanged();

                sendMessage(QJsonObject{
                                        {"cmd",   +CommandType::authorize},
                                        {"nonce", "auth_1"},
                                        {"args",  QJsonObject{
                                                     {"client_id", clientID},
                                                     {"scopes",    QJsonArray::fromStringList(scopes)}
                                                 }},
                                        });

                // The user has to confirm a dialog in Discord - give them time to do so
                const QDiscordMessage msg = readMessage(authorizeTimeout);
                if(msg.json["cmd"] != "AUTHORIZE" || msg.json["evt"] == "ERROR") {
                    connectionError_ = "ERR 4";
                    qWarning() << "AUTHORIZE ERROR" << msg.json;
                    return false;
                }

                authCode = msg.data["code"].toString();
                connectionError_ = "CONNECTING";
            }

            // Get access token
            {
                const QNetworkReply::NetworkError err = requestToken({
                                                                         {"client_id",     clientID},
                                                                         {"client_secret", clientSecret},
                                                                         {"code",          authCode},
                                                                         {"scope",         scopes.join(' ')},
                                                                         {"grant_type",    "authorization_code"},
                                                                         // Discord rejects the code exchange without the redirect URI configured for the app
                                                                         {"redirect_uri",  redirectUri},
                                                                     }, oauthData);

                if(err != QNetworkReply::NoError) {
                    connectionError_ = "ERR 5";
                    return false;
                }
            }

            if(oauthData["access_token"].toString().isEmpty()) {
                connectionError_ = "ERR 6";
                qWarning() << "QDiscord failed to obtain access token";
                return false;
            }

            saveOauthData();
        }

        // Authenticate
        if(!authenticate("auth_2")) {
            connectionError_ = "ERR 7";
            return false;
        }

        qDebug() << "Connection successful";
        return true;
    }();

    connecting_ = false;
    processing_--;

    if(!r)
        disconnect();

    else {
        isConnected_ = true;
        connectionError_.clear();
        emit connected();
    }

    emit connectionStateChanged();
    return r;
}

QString QDiscord::oauthFilePath() {
#ifdef Q_OS_MACOS
    // The executable lives in <plugin>/bin/<name>.app/Contents/MacOS, keep the token next to the plugin's manifest
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../../../../discordOauth.json");
#else
    // The executable lives in <plugin>/bin, keep the token next to the plugin's manifest
    return QDir(QCoreApplication::applicationDirPath()).absoluteFilePath("../discordOauth.json");
#endif
}

QString QDiscord::ipcPipeName(int index) {
    const QString name = QStringLiteral("discord-ipc-%1").arg(index);
#ifdef Q_OS_WIN
    // Named pipe \\.\pipe\discord-ipc-N
    return name;
#else
    // Unix socket in the first temp directory that is set, same lookup order as Discord's own RPC library
    for(const char *var: {"XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP"}) {
        const QString dir = qEnvironmentVariable(var);
        if(!dir.isEmpty())
            return QDir(dir).absoluteFilePath(name);
    }
    return QStringLiteral("/tmp/") + name;
#endif
}

QNetworkReply::NetworkError QDiscord::requestToken(const QUrlQuery &query, QJsonObject &result) {
    QNetworkRequest req{QUrl(QString::fromLatin1(tokenUrl))};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    req.setTransferTimeout(httpTimeout);

    QNetworkReply *r = netMgr_.post(req, query.toString(QUrl::FullyEncoded).toUtf8());

    QEventLoop l;
    QObject::connect(r, &QNetworkReply::finished, &l, &QEventLoop::quit);

    qDebug() << "TOKEN REQ" << req.url() << query.queryItemValue("grant_type");

    if(!r->isFinished())
        l.exec();

    r->deleteLater();

    if(r->error() != QNetworkReply::NoError) {
        qWarning() << "QDiscord Network error (token)" << r->error() << r->errorString() << r->readAll();
        return r->error();
    }

    result = QJsonDocument::fromJson(r->readAll()).object();
    return QNetworkReply::NoError;
}

void QDiscord::disconnect() {
    const bool wasConnected = isConnected_;

    socket_.disconnectFromServer();
    isConnected_ = false;
    userID_.clear();

    if(wasConnected)
        emit disconnected();
}

QDiscordReply *QDiscord::sendCommand(const QString &command, const QJsonObject &args, const QJsonObject &msgOverrides) {
    const QString nonce = QStringLiteral("%1:%2").arg(QString::number(nonceCounter_++), QString::number(QRandomGenerator64::global()->generate()));
    QJsonObject message{
        {"cmd",   command},
        {"args",  args},
        {"nonce", nonce}
    };

    for(auto it = msgOverrides.begin(), end = msgOverrides.end(); it != end; it++)
        message[it.key()] = it.value();

    QDiscordReply *r = new QDiscordReply(nonce);

    // Commands sent before the authentication finished would mess up the handshake - fail them instead
    if(!isConnected_) {
        QMetaObject::invokeMethod(r, [r, nonce] {
            emit r->finished(QDiscordMessage::fromJson(QJsonObject{{"evt", "ERROR"}, {"nonce", nonce}, {"data", QJsonObject{{"message", "Not connected"}}}}, 1));
            r->deleteLater();
        }, Qt::QueuedConnection);
        return r;
    }

    sendMessage(message);

    pendingReplies_.insert(nonce, r);
    return r;
}

QImage QDiscord::getUserAvatar(const QString &userId, const QString &avatarId) {
    if(QImage * img = avatarsCache_.object(avatarId))
        return *img;

    const QString url = QStringLiteral("https://%1/avatars/%2/%3.png").arg(cdn_, userId, avatarId);
    QNetworkReply *r = netMgr_.get(QNetworkRequest(QUrl(url)));
    QObject::connect(r, &QNetworkReply::finished, this, [this, r, avatarId] {
        const QImage img = QImage::fromData(r->readAll());
        r->deleteLater();
        avatarsCache_.insert(avatarId, new QImage(img));
        emit avatarReady(avatarId, img);
    });

    return {};
}

QDiscordMessage QDiscord::readMessage(int timeoutMs) {
    const QByteArray headerBA = blockingReadBytes(sizeof(MessageHeader), timeoutMs);
    if(headerBA.isNull()) {
        qDebug() << "Empty json message";
        return {};
    }

    const MessageHeader &header = *reinterpret_cast<const MessageHeader *>(headerBA.data());

    const QByteArray data = blockingReadBytes(static_cast<int>(header.length), timeoutMs);

    QJsonParseError err;
    QDiscordMessage result = QDiscordMessage::fromJson(QJsonDocument::fromJson(data, &err).object(), static_cast<int>(header.opcode));

    if(err.error != QJsonParseError::NoError)
        qWarning() << "QDiscord - failed to parse message\n\n" << data;

    qDebug() << "<<<<< RECV\n" << header.opcode << header.length << result.json << "\n";

    return result;
}

void QDiscord::sendMessage(const QJsonObject &packet, int opCode) {
    const QByteArray payload = QJsonDocument(packet).toJson(QJsonDocument::Compact);

    qDebug() << ">>>>> SEND\n" << opCode << payload.length() << packet << "\n";

    MessageHeader header;
    header.opcode = static_cast<uint32_t>(opCode);
    header.length = static_cast<uint32_t>(payload.length());

    socket_.write(QByteArray::fromRawData(reinterpret_cast<const char *>(&header), sizeof(MessageHeader)));
    socket_.write(payload);
}

void QDiscord::processMessage(const QDiscordMessage &msg) {
    if(QDiscordReply *r = pendingReplies_.take(msg.nonce)) {
        emit r->finished(msg);
        r->deleteLater();
        return;
    }

    emit messageReceived(msg);
}

QByteArray QDiscord::blockingReadBytes(int bytes, int timeoutMs) {
    blockingRead_++;
    processing_++;

    // Wait in an event loop rather than socket_.waitForReadyRead, so the rest of the plugin
    // (Stream Deck websocket, timers) keeps working while we wait for Discord
    if(socket_.bytesAvailable() < bytes && socket_.state() == QLocalSocket::ConnectedState) {
        QEventLoop l;
        QTimer timeout;
        timeout.setSingleShot(true);
        QObject::connect(&timeout, &QTimer::timeout, &l, &QEventLoop::quit);
        QObject::connect(&socket_, &QLocalSocket::disconnected, &l, &QEventLoop::quit);
        QObject::connect(&socket_, &QLocalSocket::readyRead, &l, [&] {
            if(socket_.bytesAvailable() >= bytes)
                l.quit();
        });
        timeout.start(timeoutMs);
        l.exec();
    }

    processing_--;
    blockingRead_--;

    if(socket_.bytesAvailable() < bytes) {
        qWarning() << "QDiscord - read timeout" << socket_.bytesAvailable() << socket_.state();
        return {};
    }

    return socket_.read(bytes);
}

void QDiscord::readAndProcessMessages() {
    while(socket_.bytesAvailable())
        processMessage(readMessage());
}

QString operator +(QDiscord::CommandType ct) {
    static const QHash<int, QString> ht = [] {
        const auto me = QMetaEnum::fromType<QDiscord::CommandType>();
        const auto cnt = me.keyCount();

        QHash<int, QString> r;
        r.reserve(cnt);

        const QRegularExpression regex("([A-Z])");

        for(int i = 0; i < cnt; i++) {
            QString str = me.key(i);
            str.replace(regex, "_\\1"); // Add _ underscore before every capitalized letter (guildStatus -> guild_Status)
            str = str.toUpper(); // Convert all to uppercase (guild_Status -> GUILD_STATUS)

            r.insert(me.value(i), str);
        }

        return r;
    }();
    return ht.value(int(ct));
}

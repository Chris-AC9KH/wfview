#ifndef TCISERVER_H
#define TCISERVER_H

#include <QObject>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QtWebSockets>

#include "audioconverter.h"
#include "cachingqueue.h"

/* Opus and Eigen */
#ifndef Q_OS_LINUX
#include "../eigen/Eigen/Eigen"
#include "opus.h"
#else
#include "opus/opus.h"
#include <eigen3/Eigen/Eigen>
#endif

#define TCI_AUDIO_LENGTH 4096
struct tciCommandStruct {
    const char *str;
    funcs func;
    valueType arg1;
    valueType arg2;
    valueType arg3;
};

class tciServer : public QObject {
    Q_OBJECT

    static constexpr quint32 iqHeaderSize = 16u * sizeof(quint32);
    static constexpr quint32 iqBufferSize = 2048u;

    typedef enum {
        IqInt16 = 0,
        IqInt24,
        IqInt32,
        IqFloat32,
        IqFloat64
    } iqDataType;

    typedef enum {
        IqStream = 0,
        RxAudioStream,
        TxAudioStream,
        TxChrono,
    } streamType;

    typedef struct {
        quint32 receiver;
        quint32 sampleRate;
        quint32 format;
        quint32 codec;
        quint32 crc;
        quint32 length;
        quint32 type;
        quint32 reserv[9];
        float data[TCI_AUDIO_LENGTH];
    } dataStream;

    struct connStatus {
        bool connected=true;
        bool rxaudio=false;
        bool txaudio=false;
        bool streamConfigured = false;   // WSJT-x broken implementation of TCI 1.4
        quint32 sampleType = IqFloat32;  // AUDIO_STREAM_SAMPLE_TYPE (2.1 spec default: float32)
        quint32 channels = 2;            // AUDIO_STREAM_CHANNELS (spec default: 2)
        quint32 samples = 2048;          // AUDIO_STREAM_SAMPLES (spec default at 48 kHz)
        quint32 rxCount = 0;             // packet counter, RX audio stream
        quint32 chronoCount = 0;         // packet counter, TX_CHRONO stream
        QByteArray rxPending;            // mono float32 waiting to fill a packet
        QByteArray txPending;            // WSJT-x broken implementation of TCI 1.4
    };

  public:
    explicit tciServer(QObject *parent = nullptr);
    ~tciServer();

  signals:
    void closed();
    void sendTCIAudio(QByteArray audio);
    void audioActiveChanged(bool active);

  public slots:
    void receiveTCIAudio(audioPacket audio);
    void receiveRigCaps(rigCapabilities *caps);
    void init(quint16 port);

  private slots:
    void onNewConnection();
    void processIncomingTextMessage(QString message);
    void processIncomingBinaryMessage(QByteArray message);
    void socketDisconnected();
    void receiveCache(cacheItem item);
    void setupTxPacket(int packetLen);

  private:
    int getValueRange(funcs func, char min = 0, char max = 0, uchar rx = 0);
    void updateAudioState();
    void audioStreamCommand(QWebSocket *client, connStatus &c, const QString &cmd, const QString &value);
    bool audioActive = false;
    QWebSocketServer *server;
    QMap<QWebSocket *, connStatus> clients;
    cachingQueue *queue;
    QByteArray rxAudioData;
    QByteArray txAudioData;
    QByteArray txChrono;
    rigCapabilities *rigCaps = Q_NULLPTR;

    QString tciMode(modeInfo m);
    modeInfo rigMode(QString mode);
    int dBmConversion = 73;
};

#endif // TCISERVER_H

#ifndef AUDIOHANDLERRTOUTPUT_H
#define AUDIOHANDLERRTOUTPUT_H

#include "audiohandlerbase.h"
#include "bytering.h"
#include "../rtaudio/RtAudio.h"

#include <memory>

class audioHandlerRtOutput : public audioHandlerBase {
    Q_OBJECT

public:
    explicit audioHandlerRtOutput(QObject* parent=nullptr);
    ~audioHandlerRtOutput() override { dispose(); } // ensure close on destruction
    QString role() const override { return QStringLiteral("Output"); }

public slots:
    void incomingAudio(audioPacket packet);

protected:
    bool openDevice() noexcept override;
    void closeDevice() noexcept override;
    virtual QAudioFormat getNativeFormat() override;
    virtual bool isFormatSupported(QAudioFormat f) override;

private slots:
    void onConverted(audioPacket pkt);

private:
    static int rtCallback(void* out, void* in, unsigned nFrames, double, RtAudioStreamStatus st, void* u);
    void handleCallbackOutput(void* out, unsigned nFrames, RtAudioStreamStatus st);

private:
    RtAudio rtaudio;
    RtAudioFormat fmt{RTAUDIO_SINT16};
    unsigned bytesPerSample{2};
    unsigned bytesPerFrame{2};
    std::unique_ptr<ByteRing> outRB;
};

#endif // AUDIOHANDLERRTOUTPUT_H

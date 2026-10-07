#ifndef __SDLSOUND_H__
#define __SDLSOUND_H__

#include <SDL2/SDL.h>
#include <xmp.h>
#include "SoundManager.h"
#include "SoundInstance.h"
#include "MusicInterface.h"
#include "Common.h"
#include <map>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>

namespace Sexy
{

class SDLSoundManager;
class SDLMusicInterface;
inline SDLMusicInterface* gSDLMusicInterface = NULL;

struct SoundBuffer
{
    Uint8* mData;
    Uint32 mLength;
    SoundBuffer() : mData(NULL), mLength(0) {}
};

struct SoundChannel
{
    SoundBuffer* mBuffer;
    Uint32 mPosition;
    float mVolume;
    float mPan;
    bool mPlaying;
    bool mLooping;
    bool mAutoRelease;

    SoundChannel()
        : mBuffer(NULL), mPosition(0), mVolume(1.0f), mPan(0.0f),
          mPlaying(false), mLooping(false), mAutoRelease(false) {}
};

class SDLSoundInstance : public SoundInstance
{
public:
    SDLSoundManager* mMgr;
    int mSoundId;
    double mVolume;
    double mPan;
    int mChannelIndex;
    bool mIsReleased;

    SDLSoundInstance(SDLSoundManager* mgr, int soundId)
        : mMgr(mgr), mSoundId(soundId), mVolume(1.0), mPan(0.0),
          mChannelIndex(-1), mIsReleased(false) {}

    virtual ~SDLSoundInstance();

    virtual void Release();
    virtual void SetBaseVolume(double theBaseVolume) {}
    virtual void SetBasePan(int theBasePan) {}
    virtual void AdjustPitch(double theNumSteps) {}
    virtual void SetVolume(double theVolume);
    virtual void SetPan(int thePosition);
    virtual bool Play(bool looping, bool autoRelease);
    virtual void Stop();
    virtual bool IsPlaying();
    virtual bool IsReleased() { return mIsReleased; }
    virtual double GetVolume() { return mVolume; }
};

class SDLSoundManager : public SoundManager
{
public:
    double mMasterVolume;
    SDL_AudioDeviceID mAudioDevice;
    SDL_AudioSpec mAudioSpec;
    std::map<unsigned int, SoundBuffer> mSoundBuffers;
    SoundChannel mChannels[MAX_CHANNELS];

    static void AudioCallback(void* userdata, Uint8* stream, int len);

    SDLSoundManager() : mMasterVolume(0.8), mAudioDevice(0)
    {
        SDL_zero(mAudioSpec);
        SDL_AudioSpec desired;
        SDL_zero(desired);
        desired.freq = 44100;
        desired.format = AUDIO_S16SYS;
        desired.channels = 2;
        desired.samples = 1024;
        desired.callback = AudioCallback;
        desired.userdata = this;

        mAudioDevice = SDL_OpenAudioDevice(NULL, 0, &desired, &mAudioSpec, 0);
        if (mAudioDevice > 0)
        {
            SDL_Log("SDLSoundManager: Audio device opened (%d Hz, %d channels)",
                mAudioSpec.freq, mAudioSpec.channels);
            SDL_PauseAudioDevice(mAudioDevice, 0);
        }
        else
        {
            SDL_Log("SDLSoundManager: Warning - SDL_OpenAudioDevice failed: %s", SDL_GetError());
        }
    }

    virtual ~SDLSoundManager()
    {
        if (mAudioDevice > 0)
        {
            SDL_CloseAudioDevice(mAudioDevice);
            mAudioDevice = 0;
        }
        ReleaseSounds();
    }

    int FindFreeChannel()
    {
        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            if (!mChannels[i].mPlaying)
                return i;
        }
        return 0; // overwrite channel 0 if all are active
    }

    virtual bool Initialized() { return (mAudioDevice > 0); }

    virtual bool LoadSound(unsigned int theSfxID, const std::string& theFilename)
    {
        if (mAudioDevice == 0)
            return true; // Never fail loading when audio device is inactive

        std::string base = theFilename;
        std::string fn = Sexy::GetFileName(base, true);
        std::string dir = Sexy::GetFileDir(base, true);
        if (dir.empty()) dir = "sounds";

        std::string candidates[8];
        candidates[0] = dir + "/" + fn + ".wav";
        candidates[1] = std::string("sounds/") + fn + ".wav";
        candidates[2] = dir + "/cached_" + fn + ".wav";
        candidates[3] = std::string("sounds/cached_") + fn + ".wav";
        candidates[4] = base;
        candidates[5] = base + ".wav";
        candidates[6] = Sexy::GetResourcePath(candidates[0]);
        candidates[7] = Sexy::GetResourcePath(candidates[2]);

        std::string foundPath = "";
        for (int i = 0; i < 8; i++)
        {
            if (!candidates[i].empty() && Sexy::FileExists(candidates[i]))
            {
                foundPath = candidates[i];
                break;
            }
        }

        if (foundPath.empty())
        {
            return true;
        }

        SDL_AudioSpec wavSpec;
        Uint8* wavBuf = NULL;
        Uint32 wavLen = 0;
        bool loaded = (SDL_LoadWAV(foundPath.c_str(), &wavSpec, &wavBuf, &wavLen) != NULL);

        if (!loaded)
        {
            // Fallback manual scanner for fmt and data chunks
            FILE* fp = fopen(foundPath.c_str(), "rb");
            if (fp)
            {
                fseek(fp, 0, SEEK_END);
                long fsize = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                if (fsize > 44)
                {
                    std::vector<uint8_t> fbuf(fsize);
                    if (fread(fbuf.data(), 1, fsize, fp) == (size_t)fsize)
                    {
                        size_t pos = 12;
                        bool hasFmt = false, hasData = false;
                        uint16_t channels = 0, bitsPerSample = 0;
                        uint32_t sampleRate = 0;
                        const uint8_t* rawData = NULL;
                        uint32_t rawDataLen = 0;

                        while (pos + 8 <= (size_t)fsize)
                        {
                            char chunkId[5] = {0};
                            memcpy(chunkId, &fbuf[pos], 4);
                            uint32_t chunkLen = *(uint32_t*)&fbuf[pos + 4];
                            if (strcmp(chunkId, "fmt ") == 0 && pos + 8 + 16 <= (size_t)fsize)
                            {
                                channels = *(uint16_t*)&fbuf[pos + 10];
                                sampleRate = *(uint32_t*)&fbuf[pos + 12];
                                bitsPerSample = *(uint16_t*)&fbuf[pos + 22];
                                hasFmt = true;
                            }
                            else if (strcmp(chunkId, "data") == 0)
                            {
                                rawData = &fbuf[pos + 8];
                                rawDataLen = std::min(chunkLen, (uint32_t)(fsize - (pos + 8)));
                                hasData = true;
                                break;
                            }
                            pos += 8 + chunkLen;
                        }

                        if (hasFmt && hasData && rawData && rawDataLen > 0)
                        {
                            wavSpec.freq = sampleRate;
                            wavSpec.format = (bitsPerSample == 16) ? AUDIO_S16SYS : AUDIO_U8;
                            wavSpec.channels = channels;
                            wavBuf = (Uint8*)malloc(rawDataLen);
                            if (wavBuf)
                            {
                                memcpy(wavBuf, rawData, rawDataLen);
                                wavLen = rawDataLen;
                                loaded = true;
                            }
                        }
                    }
                }
                fclose(fp);
            }
        }

        if (!loaded || !wavBuf || wavLen == 0)
        {
            return true;
        }

        SDL_AudioCVT cvt;
        if (SDL_BuildAudioCVT(&cvt, wavSpec.format, wavSpec.channels, wavSpec.freq,
                              mAudioSpec.format, mAudioSpec.channels, mAudioSpec.freq) < 0)
        {
            free(wavBuf);
            return true;
        }

        cvt.buf = (Uint8*)malloc(wavLen * cvt.len_mult);
        if (!cvt.buf)
        {
            free(wavBuf);
            return true;
        }

        memcpy(cvt.buf, wavBuf, wavLen);
        cvt.len = wavLen;
        SDL_ConvertAudio(&cvt);
        free(wavBuf);

        ReleaseSound(theSfxID);

        SoundBuffer buf;
        buf.mData = cvt.buf;
        buf.mLength = cvt.len_cvt;
        mSoundBuffers[theSfxID] = buf;
        return true;
    }

    virtual int LoadSound(const std::string& theFilename)
    {
        int id = GetFreeSoundId();
        if (LoadSound(id, theFilename))
            return id;
        return -1;
    }

    virtual void ReleaseSound(unsigned int theSfxID)
    {
        if (mAudioDevice > 0)
            SDL_LockAudioDevice(mAudioDevice);

        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            if (mChannels[i].mBuffer == &mSoundBuffers[theSfxID])
            {
                mChannels[i].mPlaying = false;
                mChannels[i].mBuffer = NULL;
            }
        }

        auto it = mSoundBuffers.find(theSfxID);
        if (it != mSoundBuffers.end())
        {
            if (it->second.mData)
                free(it->second.mData);
            mSoundBuffers.erase(it);
        }

        if (mAudioDevice > 0)
            SDL_UnlockAudioDevice(mAudioDevice);
    }

    virtual void SetVolume(double theVolume) { mMasterVolume = theVolume; }
    virtual bool SetBaseVolume(unsigned int theSfxID, double theBaseVolume) { return true; }
    virtual bool SetBasePan(unsigned int theSfxID, int theBasePan) { return true; }

    virtual SoundInstance* GetSoundInstance(unsigned int theSfxID)
    {
        return new SDLSoundInstance(this, theSfxID);
    }

    virtual void ReleaseSounds()
    {
        if (mAudioDevice > 0)
            SDL_LockAudioDevice(mAudioDevice);

        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            mChannels[i].mPlaying = false;
            mChannels[i].mBuffer = NULL;
        }

        for (auto& pair : mSoundBuffers)
        {
            if (pair.second.mData)
                free(pair.second.mData);
        }
        mSoundBuffers.clear();

        if (mAudioDevice > 0)
            SDL_UnlockAudioDevice(mAudioDevice);
    }

    virtual void ReleaseChannels()
    {
        StopAllSounds();
    }

    virtual double GetMasterVolume() { return mMasterVolume; }
    virtual void SetMasterVolume(double theVolume) { mMasterVolume = theVolume; }
    virtual void Flush() {}
    virtual void SetCooperativeWindow(HWND theHWnd, bool isWindowed) {}

    virtual void StopAllSounds()
    {
        if (mAudioDevice > 0)
            SDL_LockAudioDevice(mAudioDevice);

        for (int i = 0; i < MAX_CHANNELS; i++)
        {
            mChannels[i].mPlaying = false;
            mChannels[i].mBuffer = NULL;
        }

        if (mAudioDevice > 0)
            SDL_UnlockAudioDevice(mAudioDevice);
    }

    virtual int GetFreeSoundId()
    {
        for (int i = 1; i < 10000; i++)
        {
            if (mSoundBuffers.find(i) == mSoundBuffers.end())
                return i;
        }
        return 1;
    }

    virtual int GetNumSounds() { return (int)mSoundBuffers.size(); }
};

inline SDLSoundInstance::~SDLSoundInstance()
{
    Stop();
}

inline void SDLSoundInstance::Release()
{
    Stop();
    mIsReleased = true;
    delete this;
}

inline void SDLSoundInstance::SetVolume(double theVolume)
{
    mVolume = theVolume;
    if (mMgr && mMgr->mAudioDevice > 0 && mChannelIndex >= 0 && mChannelIndex < MAX_CHANNELS)
    {
        SDL_LockAudioDevice(mMgr->mAudioDevice);
        mMgr->mChannels[mChannelIndex].mVolume = (float)theVolume;
        SDL_UnlockAudioDevice(mMgr->mAudioDevice);
    }
}

inline void SDLSoundInstance::SetPan(int thePosition)
{
    mPan = thePosition;
    if (mMgr && mMgr->mAudioDevice > 0 && mChannelIndex >= 0 && mChannelIndex < MAX_CHANNELS)
    {
        SDL_LockAudioDevice(mMgr->mAudioDevice);
        mMgr->mChannels[mChannelIndex].mPan = (float)thePosition;
        SDL_UnlockAudioDevice(mMgr->mAudioDevice);
    }
}

inline bool SDLSoundInstance::Play(bool looping, bool autoRelease)
{
    if (!mMgr || mMgr->mAudioDevice == 0)
        return false;

    auto it = mMgr->mSoundBuffers.find(mSoundId);
    if (it == mMgr->mSoundBuffers.end() || !it->second.mData)
        return false;

    SDL_LockAudioDevice(mMgr->mAudioDevice);
    int chIdx = mMgr->FindFreeChannel();
    if (chIdx >= 0)
    {
        SoundChannel& ch = mMgr->mChannels[chIdx];
        ch.mBuffer = &it->second;
        ch.mPosition = 0;
        ch.mVolume = (float)mVolume;
        ch.mPan = (float)mPan;
        ch.mLooping = looping;
        ch.mAutoRelease = autoRelease;
        ch.mPlaying = true;
        mChannelIndex = chIdx;
    }
    SDL_UnlockAudioDevice(mMgr->mAudioDevice);
    return true;
}

inline void SDLSoundInstance::Stop()
{
    if (mMgr && mMgr->mAudioDevice > 0 && mChannelIndex >= 0 && mChannelIndex < MAX_CHANNELS)
    {
        SDL_LockAudioDevice(mMgr->mAudioDevice);
        if (mMgr->mChannels[mChannelIndex].mPlaying)
        {
            mMgr->mChannels[mChannelIndex].mPlaying = false;
            mMgr->mChannels[mChannelIndex].mBuffer = NULL;
        }
        SDL_UnlockAudioDevice(mMgr->mAudioDevice);
    }
}

inline bool SDLSoundInstance::IsPlaying()
{
    if (mMgr && mChannelIndex >= 0 && mChannelIndex < MAX_CHANNELS)
    {
        return mMgr->mChannels[mChannelIndex].mPlaying;
    }
    return false;
}

struct XmpSong
{
    xmp_context mCtx;
    std::string mFileName;
    bool mLoaded;
    bool mPlaying;
    bool mPaused;
    bool mNoLoop;
    int mOffset;
    double mVolume;
    double mFadeSpeed;
    int mFadeDir;
    bool mStopOnFade;

    XmpSong()
        : mCtx(NULL), mLoaded(false), mPlaying(false), mPaused(false),
          mNoLoop(false), mOffset(0), mVolume(1.0), mFadeSpeed(0.0),
          mFadeDir(0), mStopOnFade(false)
    {
    }
};

class SDLMusicInterface : public MusicInterface
{
public:
    double mMasterVolume;
    SDL_mutex* mMutex;
    std::map<int, XmpSong> mSongs;
    std::vector<int16_t> mMixBuffer;

    SDLMusicInterface()
        : mMasterVolume(0.8), mMutex(NULL)
    {
        mMutex = SDL_CreateMutex();
        gSDLMusicInterface = this;
    }

    virtual ~SDLMusicInterface()
    {
        if (gSDLMusicInterface == this)
            gSDLMusicInterface = NULL;

        if (mMutex)
        {
            SDL_LockMutex(mMutex);
            for (auto& pair : mSongs)
            {
                if (pair.second.mCtx)
                {
                    xmp_end_player(pair.second.mCtx);
                    xmp_release_module(pair.second.mCtx);
                    xmp_free_context(pair.second.mCtx);
                    pair.second.mCtx = NULL;
                }
            }
            mSongs.clear();
            SDL_UnlockMutex(mMutex);
            SDL_DestroyMutex(mMutex);
            mMutex = NULL;
        }
    }

    virtual bool LoadMusic(int theSongId, const std::string& theFileName)
    {
        if (!mMutex) return false;
        SDL_LockMutex(mMutex);

        auto it = mSongs.find(theSongId);
        if (it != mSongs.end())
        {
            if (it->second.mCtx)
            {
                xmp_end_player(it->second.mCtx);
                xmp_release_module(it->second.mCtx);
                xmp_free_context(it->second.mCtx);
                it->second.mCtx = NULL;
            }
        }

        std::string foundPath = theFileName;
        if (!Sexy::FileExists(foundPath))
        {
            std::string candidates[4];
            candidates[0] = Sexy::GetResourcePath(theFileName);
            candidates[1] = "music/zuma.it";
            candidates[2] = Sexy::GetResourcePath("music/zuma.it");
            candidates[3] = "music/zuma.mo3";
            for (int i = 0; i < 4; i++)
            {
                if (!candidates[i].empty() && Sexy::FileExists(candidates[i]))
                {
                    foundPath = candidates[i];
                    break;
                }
            }
        }

        xmp_context ctx = xmp_create_context();
        if (!ctx)
        {
            SDL_Log("SDLMusicInterface: Failed to create xmp context for song %d", theSongId);
            SDL_UnlockMutex(mMutex);
            return false;
        }

        int ret = xmp_load_module(ctx, foundPath.c_str());
        if (ret != 0)
        {
            SDL_Log("SDLMusicInterface: xmp_load_module failed (%d) for %s", ret, foundPath.c_str());
            xmp_free_context(ctx);
            SDL_UnlockMutex(mMutex);
            return false;
        }

        xmp_start_player(ctx, 44100, 0);

        XmpSong& song = mSongs[theSongId];
        song.mCtx = ctx;
        song.mFileName = foundPath;
        song.mLoaded = true;
        song.mPlaying = false;
        song.mPaused = false;
        song.mVolume = 1.0;
        song.mFadeDir = 0;

        SDL_Log("SDLMusicInterface: Loaded song %d from %s", theSongId, foundPath.c_str());
        SDL_UnlockMutex(mMutex);
        return true;
    }

    virtual void PlayMusic(int theSongId, int theOffset = 0, bool noLoop = false)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        auto it = mSongs.find(theSongId);
        if (it != mSongs.end() && it->second.mLoaded && it->second.mCtx)
        {
            XmpSong& song = it->second;
            song.mOffset = theOffset;
            song.mNoLoop = noLoop;
            song.mVolume = 1.0;
            song.mFadeDir = 0;
            song.mPaused = false;
            xmp_restart_module(song.mCtx);
            if (theOffset >= 0)
            {
                xmp_set_position(song.mCtx, theOffset);
            }
            xmp_play_buffer(song.mCtx, NULL, 0, 0);
            song.mPlaying = true;
            SDL_Log("SDLMusicInterface: PlayMusic song=%d offset=%d noLoop=%d", theSongId, theOffset, (int)noLoop);
        }
        else
        {
            SDL_Log("SDLMusicInterface: PlayMusic song=%d not loaded", theSongId);
        }

        SDL_UnlockMutex(mMutex);
    }

    virtual void StopMusic(int theSongId)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        auto it = mSongs.find(theSongId);
        if (it != mSongs.end())
        {
            it->second.mPlaying = false;
            it->second.mPaused = false;
            it->second.mFadeDir = 0;
            if (it->second.mCtx)
                xmp_stop_module(it->second.mCtx);
        }

        SDL_UnlockMutex(mMutex);
    }

    virtual void PauseMusic(int theSongId)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        auto it = mSongs.find(theSongId);
        if (it != mSongs.end()) it->second.mPaused = true;
        SDL_UnlockMutex(mMutex);
    }

    virtual void ResumeMusic(int theSongId)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        auto it = mSongs.find(theSongId);
        if (it != mSongs.end()) it->second.mPaused = false;
        SDL_UnlockMutex(mMutex);
    }

    virtual void StopAllMusic()
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        for (auto& pair : mSongs)
        {
            pair.second.mPlaying = false;
            pair.second.mPaused = false;
            pair.second.mFadeDir = 0;
            if (pair.second.mCtx)
                xmp_stop_module(pair.second.mCtx);
        }

        SDL_UnlockMutex(mMutex);
    }

    virtual void PauseAllMusic()
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        for (auto& pair : mSongs) pair.second.mPaused = true;
        SDL_UnlockMutex(mMutex);
    }

    virtual void ResumeAllMusic()
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        for (auto& pair : mSongs) pair.second.mPaused = false;
        SDL_UnlockMutex(mMutex);
    }

    virtual void UnloadMusic(int theSongId)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        auto it = mSongs.find(theSongId);
        if (it != mSongs.end())
        {
            if (it->second.mCtx)
            {
                xmp_end_player(it->second.mCtx);
                xmp_release_module(it->second.mCtx);
                xmp_free_context(it->second.mCtx);
                it->second.mCtx = NULL;
            }
            mSongs.erase(it);
        }
        SDL_UnlockMutex(mMutex);
    }

    virtual void UnloadAllMusic()
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        for (auto& pair : mSongs)
        {
            if (pair.second.mCtx)
            {
                xmp_end_player(pair.second.mCtx);
                xmp_release_module(pair.second.mCtx);
                xmp_free_context(pair.second.mCtx);
                pair.second.mCtx = NULL;
            }
        }
        mSongs.clear();
        SDL_UnlockMutex(mMutex);
    }

    virtual void FadeIn(int theSongId, int theOffset = -1, double theSpeed = 0.002, bool noLoop = false)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        auto it = mSongs.find(theSongId);
        if (it != mSongs.end() && it->second.mLoaded && it->second.mCtx)
        {
            XmpSong& song = it->second;
            if (theOffset >= 0)
            {
                song.mOffset = theOffset;
                xmp_restart_module(song.mCtx);
                xmp_set_position(song.mCtx, theOffset);
                xmp_play_buffer(song.mCtx, NULL, 0, 0);
            }
            song.mNoLoop = noLoop;
            song.mVolume = 0.0;
            song.mFadeSpeed = (theSpeed > 0.0) ? theSpeed : 0.002;
            song.mFadeDir = 1;
            song.mPlaying = true;
            song.mPaused = false;
            SDL_Log("SDLMusicInterface: FadeIn song=%d offset=%d speed=%f", theSongId, theOffset, theSpeed);
        }

        SDL_UnlockMutex(mMutex);
    }

    virtual void FadeOut(int theSongId, bool stopSong = true, double theSpeed = 0.004)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        auto it = mSongs.find(theSongId);
        if (it != mSongs.end() && it->second.mLoaded && it->second.mCtx)
        {
            XmpSong& song = it->second;
            song.mFadeSpeed = (theSpeed > 0.0) ? theSpeed : 0.004;
            song.mFadeDir = -1;
            song.mStopOnFade = stopSong;
            SDL_Log("SDLMusicInterface: FadeOut song=%d stopSong=%d speed=%f", theSongId, (int)stopSong, theSpeed);
        }

        SDL_UnlockMutex(mMutex);
    }

    virtual void FadeOutAll(bool stopSong = true, double theSpeed = 0.004)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        for (auto& pair : mSongs)
        {
            XmpSong& song = pair.second;
            if (song.mPlaying)
            {
                song.mFadeSpeed = (theSpeed > 0.0) ? theSpeed : 0.004;
                song.mFadeDir = -1;
                song.mStopOnFade = stopSong;
            }
        }

        SDL_UnlockMutex(mMutex);
    }

    virtual void SetVolume(double theVolume)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        mMasterVolume = theVolume;
        SDL_UnlockMutex(mMutex);
    }

    virtual void SetSongVolume(int theSongId, double theVolume)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);
        auto it = mSongs.find(theSongId);
        if (it != mSongs.end())
        {
            it->second.mVolume = theVolume;
        }
        SDL_UnlockMutex(mMutex);
    }

    virtual bool IsPlaying(int theSongId)
    {
        if (!mMutex) return false;
        SDL_LockMutex(mMutex);
        bool res = false;
        auto it = mSongs.find(theSongId);
        if (it != mSongs.end())
        {
            res = (it->second.mPlaying && !it->second.mPaused);
        }
        SDL_UnlockMutex(mMutex);
        return res;
    }

    virtual void Update()
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        for (auto& pair : mSongs)
        {
            XmpSong& song = pair.second;
            if (song.mFadeDir > 0)
            {
                song.mVolume += song.mFadeSpeed;
                if (song.mVolume >= 1.0)
                {
                    song.mVolume = 1.0;
                    song.mFadeDir = 0;
                }
            }
            else if (song.mFadeDir < 0)
            {
                song.mVolume -= song.mFadeSpeed;
                if (song.mVolume <= 0.0)
                {
                    song.mVolume = 0.0;
                    song.mFadeDir = 0;
                    if (song.mStopOnFade)
                    {
                        song.mPlaying = false;
                        if (song.mCtx)
                            xmp_stop_module(song.mCtx);
                    }
                }
            }
        }

        SDL_UnlockMutex(mMutex);
    }

    void MixAudio(Uint8* stream, int len, SDL_AudioFormat format)
    {
        if (!mMutex) return;
        SDL_LockMutex(mMutex);

        if (mMasterVolume <= 0.0)
        {
            SDL_UnlockMutex(mMutex);
            return;
        }

        if ((int)mMixBuffer.size() * (int)sizeof(int16_t) < len)
        {
            mMixBuffer.resize((len / sizeof(int16_t)) + 64);
        }

        for (auto& pair : mSongs)
        {
            XmpSong& song = pair.second;
            if (!song.mPlaying || song.mPaused || !song.mCtx)
                continue;

            double effVol = mMasterVolume * song.mVolume;
            int vol = (int)(effVol * SDL_MIX_MAXVOLUME);
            if (vol <= 0)
                continue;
            if (vol > SDL_MIX_MAXVOLUME)
                vol = SDL_MIX_MAXVOLUME;

            memset(mMixBuffer.data(), 0, len);
            int ret = xmp_play_buffer(song.mCtx, mMixBuffer.data(), len, song.mNoLoop ? 1 : 0);
            if (ret < 0 || (song.mNoLoop && ret != 0))
            {
                song.mPlaying = false;
            }

            SDL_MixAudioFormat(stream, (const Uint8*)mMixBuffer.data(), format, len, vol);
        }

        SDL_UnlockMutex(mMutex);
    }
};

inline void SDLSoundManager::AudioCallback(void* userdata, Uint8* stream, int len)
{
    SDLSoundManager* mgr = (SDLSoundManager*)userdata;
    memset(stream, 0, len);

    if (gSDLMusicInterface != NULL)
    {
        gSDLMusicInterface->MixAudio(stream, len, mgr->mAudioSpec.format);
    }

    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        SoundChannel& ch = mgr->mChannels[i];
        if (!ch.mPlaying || !ch.mBuffer || !ch.mBuffer->mData)
            continue;

        Uint32 remaining = ch.mBuffer->mLength - ch.mPosition;
        Uint32 toMix = (remaining > (Uint32)len) ? (Uint32)len : remaining;
        int vol = (int)(ch.mVolume * mgr->mMasterVolume * SDL_MIX_MAXVOLUME);
        if (vol > SDL_MIX_MAXVOLUME) vol = SDL_MIX_MAXVOLUME;
        if (vol < 0) vol = 0;

        if (vol > 0 && toMix > 0)
        {
            SDL_MixAudioFormat(stream, ch.mBuffer->mData + ch.mPosition, mgr->mAudioSpec.format, toMix, vol);
        }

        ch.mPosition += toMix;
        if (ch.mPosition >= ch.mBuffer->mLength)
        {
            if (ch.mLooping)
            {
                ch.mPosition = 0;
            }
            else
            {
                ch.mPlaying = false;
                ch.mBuffer = NULL;
            }
        }
    }
}

} // namespace Sexy

#endif

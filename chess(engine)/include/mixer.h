#ifndef MIXER_H
#define MIXER_H


class mixer
{
    public:
        mixer();
        ~mixer();
        void playDown();
    protected:
        Mix_Chunk* down;
    private:
};
mixer::mixer()
{
    Mix_Init(MIX_INIT_MOD);
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 768);
    down = Mix_LoadWAV("sounds\\down.wav");
    Mix_VolumeChunk(down, 60);
}
mixer::~mixer()
{
    Mix_FreeChunk(down);
    Mix_Quit();
}
void mixer::playDown()
{
    Mix_PlayChannel(-1, down, 0);
}
#endif // MIXER_H

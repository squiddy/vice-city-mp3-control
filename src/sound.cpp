#include "sound.h"

void InitGameSound(GameSound* pSound)
{
	pSound->CloseStream = *(p_AIL_close_stream*)ADDR_AIL_CLOSE_STREAM;
	pSound->StreamMsPosition = *(p_AIL_stream_ms_position*)ADDR_AIL_STREAM_MS_POSITION;
	pSound->SetStreamMsPosition = *(p_AIL_set_stream_ms_position*)ADDR_AIL_SET_STREAM_MS_POSITION;
	pSound->OpenStream = *(p_AIL_open_stream*)ADDR_AIL_OPEN_STREAM;
	pSound->PauseStream = *(p_AIL_pause_stream*)ADDR_AIL_PAUSE_STREAM;
}
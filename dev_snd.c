#include <stdio.h> 
#include <stdlib.h>  
#include <alsa/asoundlib.h>  
   
int play_sound(char*file)  
{  
    int i,ret;  
    int buf[128];  
    unsigned int val;  
    int dir=0;  
    char *buffer;  
    int size;  
    snd_pcm_uframes_t frames;  
    snd_pcm_uframes_t periodsize;  
    snd_pcm_t *playback_handle;				//pcm device handle
    snd_pcm_hw_params_t *hw_params;			//hdware info/config & pcm stream info  
    FILE *fp = fopen(file, "rb");  
    if(fp==NULL)  
    	return 0;  
    fseek(fp,100,SEEK_SET);  
    if((ret=snd_pcm_open(&playback_handle,"default", SND_PCM_STREAM_PLAYBACK, 0))<0){
        perror("snd_pcm_open");  
        exit(1);  
    }  
       
    if((ret=snd_pcm_hw_params_malloc(&hw_params))<0){  
        perror("snd_pcm_hw_params_malloc");  
        exit(1);  
    }  
    if((ret=snd_pcm_hw_params_any(playback_handle, hw_params))<0){  
        perror("snd_pcm_hw_params_any");  
        exit(1);  
    }  
	/*init access rights*/
    ret = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);  
    if (ret < 0) {  
        perror("snd_pcm_hw_params_set_access");  
        exit(1);  
    }  
    /* init sample format SND_PCM_FORMAT_U8,8λ  */
   if((ret=snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_U8))<0){  
        perror("snd_pcm_hw_params_set_format");  
        exit(1);  
    }  
   /*init sample rate : 44.1kHz & dir:play*/
    val = 44100;  
    ret = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &val, &dir);  
    if (ret < 0) {  
        perror("snd_pcm_hw_params_set_rate_near");  
        exit(1);  
    }  
    /*set channels: stereo*/
    ret = snd_pcm_hw_params_set_channels(playback_handle, hw_params, 2);  
    if (ret < 0) {  
        perror("snd_pcm_hw_params_set_channels");  
        exit(1);  
    }  
       
    /* Set period size to 32 frames. */  
    frames = 32;  
    periodsize = frames * 2;  
    ret = snd_pcm_hw_params_set_buffer_size_near(playback_handle, hw_params, &periodsize);  
    if (ret < 0){  
         printf("Unable to set buffer size %li : %s\n", frames * 2, snd_strerror(ret));  
            
    }  
         periodsize /= 2;  
   
    ret = snd_pcm_hw_params_set_period_size_near(playback_handle, hw_params, &periodsize, 0);  
    if (ret < 0)   
    {  
        printf("Unable to set period size %li : %s\n", periodsize,  snd_strerror(ret));  
    }  
                                     
    /*now set the para*/
    ret = snd_pcm_hw_params(playback_handle, hw_params);  
    if (ret < 0) {  
        perror("snd_pcm_hw_params");  
        exit(1);  
    }  
       
     /* Use a buffer large enough to hold one period */  
    snd_pcm_hw_params_get_period_size(hw_params, &frames, &dir);  
                                   
    size = frames * 2; /* 2 bytes/sample, 2 channels */  
    buffer = (char *) malloc(size);  
    fprintf(stderr, "size = %d\n", size);  
       
    while (1)   
    {  
        ret = fread(buffer, 1, size, fp);  
        if(ret==0){  
              fprintf(stderr, "end of file on input\n");  
              break;  
        }
        while(ret = snd_pcm_writei(playback_handle, buffer, frames)<0){  
            usleep(2000);  
            if (ret == -EPIPE){  
                  /* EPIPE means underrun */  
                  fprintf(stderr, "underrun occurred\n");  
                  //re prepare  
                  snd_pcm_prepare(playback_handle);  
            }else{  
                  fprintf(stderr,"error from writei: %s\n", snd_strerror(ret));  
            }    
        }  
           
    }         
    /*close dev handle*/
    snd_pcm_close(playback_handle);  
    return 0;  
}  

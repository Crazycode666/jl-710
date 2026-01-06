#ifndef __LLNS_H_
#define __LLNS_H_

typedef struct {
    float gainfloor;
    float suppress_level;
    float noise_level;
} llns_param_t;

int JLSP_llns_get_heap_size(int *share_head_size, int *private_head_size, int samplerate, int bitw);

void *JLSP_llns_init(char *private_buffer, int private_size, char *share_buffer, int share_size, int samplerate, float gainfloor, float suppress_level, int bitw);

int JLSP_llns_reset(void *m);

int JLSP_llns_process(void *m, void *pcm, void *outbuf, int *outsize);

int JLSP_llns_free(void *m);

int JLSP_llns_set_parameters(void *m, llns_param_t *p);

int JLSP_llns_set_winsize(void *m, int winsize);

int JLSP_llns_set_subframe_div(void *m, int div);

void JLSP_llns_set_noiselevel(void *m, float noise_level_init);


#endif

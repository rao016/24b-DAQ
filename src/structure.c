#include "structure.h"

dSet *queue = NULL;

/******************************************************************
 *
 * Description: Adds a sample set to the queue
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t add(uint32_t n, float rate, uint16_t c) {
    dSet *temp = (dSet*) malloc(sizeof(dSet));
    dSet *end = queue;
    
	if (temp == NULL) return FULL_RESPONSE;
    else if (n > 0 && rate > MIN_RATE && rate < MAX_RATE && c > 0 && c <= 0b00111111) {
        //Number of Samples
        temp->num = n;
        //Channels
        temp->channels = c;
		//Sample Rate
		temp->rate = rate;
        //Pointer to next
        temp->next = NULL;
        
        if (queue == NULL) queue = temp;
        else {
            while(end->next != NULL) end = end->next;
            end->next = temp;
        }
        return OK_RESPONSE;
    }
    else return INVALID_RESPONSE;
}

/******************************************************************
 *
 * Description: Removes the first sample set in the queue
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t rm(void) {
    dSet *temp = queue;
	if (queue == NULL) return 0;
    queue = queue->next;
    free(temp);
    return (queue == NULL) ? 0 : 1;
}

/******************************************************************
 *
 * Description: Decrements the number of samples to be taken in the
 *  first sample set
 * Last Modified: 11/1/17
 *
 ******************************************************************/
uint8_t dec(void) {
    if (queue == NULL) return 0;
    else if (--(queue->num) == 0) return rm() ? 2 : 0;
    else return 1;
}

/******************************************************************
 *
 * Description: Returns the sample set at a given number.  Returns
 *  null if it doesn't exist
 * Last Modified: 11/1/17
 *
 ******************************************************************/
dSet* findSet(uint32_t n) {
    uint32_t i;
    dSet *temp = queue;
    
    for(i = 0; i < n && temp != NULL; i++) temp = temp->next;
    
    return temp;
}

/******************************************************************
 *
 * Description: Prints sample set information into the buf variable
 *  of the sample set at position 'ss'
 * Last Modified: 11/1/17
 *
 ******************************************************************/
dSet* qryDSet(uint32_t ss, char *buf, uint32_t buf_len) {
	dSet *temp = findSet(ss);
    
    if (temp != NULL) snprintf(buf, buf_len, "Number of Samples: %lu\tSample Rate: %f\tChannels:%u\n", (uint32_t) temp->num, temp->rate, temp->channels);
	else strcpy(buf,"Does Not Exist");
    return temp;
}

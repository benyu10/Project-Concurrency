#include "channel.h"

// Creates a new channel with the provided size and returns it to the caller
// A 0 size indicates an unbuffered channel, whereas a positive size indicates a buffered channel
channel_t* channel_create(size_t size)
{
    /* IMPLEMENT THIS */

    //if (size >= 1) //positive size, so it is a buffered 
        //{
        //channel->buffer = buffer_create(size); //buffer create function from slides
        //channel->closed = false;
        //}

    //creating new channel with size
    size_t* c_size = malloc(sizeof(channel_t)); //gets the size of struct
    channel_t* chan = (channel_t*) c_size;
    
    pthread_mutex_init(&chan->mutex, NULL);
    
    pthread_cond_init(&chan->send, NULL);
    pthread_cond_init(&chan->receive, NULL);

    if (size >= 1) //positive size, so it is a buffered
    {
        chan->buffer = buffer_create(size); //buffer create function from slides
        chan->channel_close = false;
    }
    
    return chan;
}

// Writes data to the given channel
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the channel is full, the function waits till the channel has space to write the new data
// Returns SUCCESS for successfully writing data to the channel,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_send(channel_t *channel, void* data)
{
    /* IMPLEMENT THIS */

    pthread_mutex_lock(&channel->mutex);

    //size_t b_size = buffer_current_size(channel->buffer);
    //size_t b_capacity = buffer_capacity(channel->buffer);
    
    if (channel->channel_close == false) //means that it isnt closed, then we have to wait
    {
        while (buffer_capacity(channel->buffer) <=  buffer_current_size(channel->buffer)) //if there is less space for the size, then we have to wait
        {
         
            if (channel->channel_close == true) //try
            { //passes close on send
                pthread_mutex_unlock(&channel->mutex);
                return CLOSED_ERROR; //pthread_cond_wait(&channel->send, &channel->mutex);
            }
            else
            {
                pthread_cond_wait(&channel->send, &channel->mutex);
            }
        }

        buffer_add(channel->buffer, data);

        pthread_cond_signal(&channel->receive);

        pthread_mutex_unlock(&channel->mutex);
        
        return SUCCESS;   
    }
    
    if (channel->channel_close == true) //channel is closed, so we return as stated in comments
    {
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    else
    {
        return GEN_ERROR;
    }
}

// Reads data from the given channel and stores it in the function's input parameter, data (Note that it is a double pointer)
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the channel is empty, the function waits till the channel has some data to read
// Returns SUCCESS for successful retrieval of data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */

     pthread_mutex_lock(&channel->mutex);

     if (channel->channel_close == false) //retrieve the data when it is not closed
    {
        //size_t b_size = buffer_current_size(channel->buffer);

        while (buffer_current_size(channel->buffer) < 1 ) //less than one means it is empty, so we wait
        {
            if (channel->channel_close == true) //try
            { //passes close on recv
                pthread_mutex_unlock(&channel->mutex);
                return CLOSED_ERROR; 
            }
            else
            {
                pthread_cond_wait(&channel->receive, &channel->mutex);
            }

            //pthread_cond_wait(&channel->receive,&channel->mutex);
        }

        buffer_remove(channel->buffer,data); //opposite of send function

        pthread_cond_signal(&channel->send);

        pthread_mutex_unlock(&channel->mutex);

        return SUCCESS;
    }

     
     if (channel->channel_close == true) //it is cllsoed, so return closed error
    {
        pthread_mutex_unlock(&channel->mutex);
        
        return CLOSED_ERROR;
    }
    else
    {
        return GEN_ERROR;

    }
}
// Writes data to the given channel
// This is a non-blocking call i.e., the function simply returns if the channel is full
// Returns SUCCESS for successfully writing data to the channel,
// CHANNEL_FULL if the channel is full and the data was not added to the buffer,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_send(channel_t* channel, void* data)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex);
    //size_t b_size = buffer_current_size(channel->buffer);
    //size_t b_capacity = buffer_capacity(channel->buffer);

    //exaclty like send, but just retuirn channel when full 
    if (channel->channel_close == true) //closed so unlocka nd reutrn
    {
        pthread_mutex_unlock(&channel->mutex);

        return CLOSED_ERROR;
    }
    
    if (channel->channel_close == false) //channel is still open, so we check if full, if not then we add
    {
        //sem_post(channel->sem);

        if (buffer_capacity(channel->buffer) <= buffer_current_size(channel->buffer)) //when space is less than size, then it is full
        {
            pthread_mutex_unlock(&channel->mutex);
            return CHANNEL_FULL;
        }
        //sem_post(channel->sem);
        else
        {
            buffer_add(channel->buffer,data);
            pthread_cond_signal(&channel->receive);
            pthread_mutex_unlock(&channel->mutex);
            return SUCCESS;
        }
    }
    
    else
    {
        return GEN_ERROR;
    }
    
}

// Reads data from the given channel and stores it in the function's input parameter data (Note that it is a double pointer)
// This is a non-blocking call i.e., the function simply returns if the channel is empty
// Returns SUCCESS for successful retrieval of data,
// CHANNEL_EMPTY if the channel is empty and nothing was stored in data,
// CLOSED_ERROR if the channel is closed, and
// GEN_ERROR on encountering any other generic error of any sort
enum channel_status channel_non_blocking_receive(channel_t* channel, void** data)
{
    /* IMPLEMENT THIS */
     pthread_mutex_lock(&channel->mutex);
      
     if (channel->channel_close == true) //followed thge comment
     {   
        pthread_mutex_unlock(&channel->mutex);

        return CLOSED_ERROR;
     }

     if (channel->channel_close == false) //check to see if channel is full, if not we store
     {

        if (buffer_current_size(channel->buffer) < 1 )
        { //if less than one then it is empty, so we return channel empty
            pthread_mutex_unlock(&channel->mutex);
            return CHANNEL_EMPTY;
        }
        
        else
        {

            buffer_remove(channel->buffer,data);    
            pthread_cond_signal(&channel->send);
            pthread_mutex_unlock(&channel->mutex);
            return SUCCESS;
        }
    }

    else
    {
        return GEN_ERROR;
    }
}

// Closes the channel and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the channel is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns SUCCESS if close is successful,
// CLOSED_ERROR if the channel is already closed, and
// GEN_ERROR in any other error case
enum channel_status channel_close(channel_t* channel)
{
    /* IMPLEMENT THIS */
    pthread_mutex_lock(&channel->mutex); //add lock

    if (channel->channel_close == false) //if channel is open, we close it by releasing all. use broadcast
    {
        channel->channel_close = true;
        pthread_cond_broadcast(&channel->send);
        pthread_cond_broadcast(&channel->receive);
        pthread_mutex_unlock(&channel->mutex);

        return SUCCESS;
    }
    if (channel->channel_close == true) //if its closed already, just return error
    {
        pthread_mutex_unlock(&channel->mutex);
        return CLOSED_ERROR;
    }
    
    else
    {    
        return GEN_ERROR;
    }

}

// Frees all the memory allocated to the channel
// The caller is responsible for calling channel_close and waiting for all threads to finish their tasks before calling channel_destroy
// Returns SUCCESS if destroy is successful,
// DESTROY_ERROR if channel_destroy is called on an open channel, and
// GEN_ERROR in any other error case
enum channel_status channel_destroy(channel_t* channel)
{
    /* IMPLEMENT THIS */

     pthread_mutex_lock(&channel->mutex);
    
    //when not closed, we unlock mutex and free all memeory allocated
    if (channel->channel_close == true)
    {
        pthread_mutex_unlock(&channel->mutex); //unlocks

        buffer_free(channel->buffer); //opposite of create

        pthread_cond_destroy(&channel->send); //opposite of cond init
        pthread_cond_destroy(&channel->receive); //opposite of cond init

        pthread_mutex_destroy(&channel->mutex); //opposite of mutex init
        free(channel); //opposite of malloc

        return SUCCESS;
    }
    //called on open channel, we unlock and return destroy error 
    if (channel->channel_close == false)
    {
        pthread_mutex_unlock(&channel->mutex);
        return DESTROY_ERROR;
    }
    
    else
    {
        return GEN_ERROR;
    }
    
}

// Takes an array of channels (channel_list) of type select_t and the array length (channel_count) as inputs
// This API iterates over the provided list and finds the set of possible channels which can be used to invoke the required operation (send or receive) specified in select_t
// If multiple options are available, it selects the first option and performs its corresponding action
// If no channel is available, the call is blocked and waits till it finds a channel which supports its required operation
// Once an operation has been successfully performed, select should set selected_index to the index of the channel that performed the operation and then return SUCCESS
// In the event that a channel is closed or encounters any error, the error should be propagated and returned through select
// Additionally, selected_index is set to the index of the channel that generated the error
enum channel_status channel_select(select_t* channel_list, size_t channel_count, size_t* selected_index)
{
    /* IMPLEMENT THIS */

    sem_t sem;
    sem_init(&sem, 0, 0);
    while (true) //waits for sem_wait
    {
        for (int i = 0; i < channel_count; i++) //iterate channel list, and looks at the individual channels
        {
            if (channel_list[i].dir == SEND) //when the operation is SEND
            {
                int operation = channel_non_blocking_send(channel_list[i].channel, &(channel_list[i].data)); 
                //non blocking, simply leave without sending
                if (operation == SUCCESS) //we just do the operation if its a success
                {
                    *selected_index = (size_t)i; //notes
                    return operation; //doing the operation
                }
                else
                {
                    //sem_wait(&sem); //wait until 
                    *selected_index = (size_t)i;
                    return operation; //doing the operation
                }
            }
            else //if not send, then it is recv
            {
                return GEN_ERROR;
            }
        //sem_wait(&sem);
    
        }
        sem_wait(&sem);
    }
    return GEN_ERROR;
}

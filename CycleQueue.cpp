/*
 * CycleQueue.cpp
 *
 *  Created on: Aug 1, 2018
 *      Author: jhb
 */

#include "CycleQueue.h"
#include <iostream>
using namespace std;

CycleQueue::CycleQueue(DP_U32 size) {
	m_cycleQueue = new CycleQueue_S();
	m_buffer = new DP_U8[size];
	pthread_mutex_init(&mutex, NULL);
//	mutex=PTHREAD_MUTEX_INITIALIZER;
	memset(m_cycleQueue, 0, sizeof(CycleQueue_S));
	memset(m_buffer, 0, size);
	queueInit(size);
}

CycleQueue::~CycleQueue() {
	delete m_cycleQueue;
	delete[] m_buffer;
	m_cycleQueue = NULL;
	m_buffer = NULL;
}

DP_U32 CycleQueue::DSP_PutData(const DP_U8 *buff, DP_U32 size) {

	DP_U32 ret = 0;
	pthread_mutex_lock(m_cycleQueue->_mutex);

	DP_U32 len = 0;
	size = minimum(size,
			m_cycleQueue->_size - m_cycleQueue->_in + m_cycleQueue->_out);
	/* first put the data starting from fifo->in to buffer end */
	len = minimum(size,
			m_cycleQueue->_size
					- (m_cycleQueue->_in & (m_cycleQueue->_size - 1)));
	memcpy(
			m_cycleQueue->_buffer
					+ (m_cycleQueue->_in & (m_cycleQueue->_size - 1)), buff,
			len);
	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(m_cycleQueue->_buffer, buff + len, size - len);
	m_cycleQueue->_in += size;
	ret = size;

	pthread_mutex_unlock(m_cycleQueue->_mutex);
	return ret;

}
DP_U32 CycleQueue::DSP_GetDate(char *buff, DP_U32 len) {
	DP_U32 ret = 0;
	pthread_mutex_lock(m_cycleQueue->_mutex);
	DP_U32 size = 0;
	len = minimum(len, m_cycleQueue->_in - m_cycleQueue->_out);
	/* first get the data from fifo->out until the end of the buffer */
	size = minimum(len,
			m_cycleQueue->_size
					- (m_cycleQueue->_out & (m_cycleQueue->_size - 1)));
	memcpy(buff,
			m_cycleQueue->_buffer
					+ (m_cycleQueue->_out & (m_cycleQueue->_size - 1)), size);
	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(buff + size, m_cycleQueue->_buffer, len - size);
	m_cycleQueue->_out += len;

	if (m_cycleQueue->_in == m_cycleQueue->_out)
		m_cycleQueue->_in = m_cycleQueue->_out = 0;

	ret = len;

	pthread_mutex_unlock(m_cycleQueue->_mutex);
	return ret;
}

DP_Bool CycleQueue::queueInit(DP_U32 size) {
	if (!isPowerOfTwo(size)) {
		cout << "Size input must be power of 2. " << endl;
		return false;
	}
	if (!m_cycleQueue) {
		cout << "Can not allocate memory of struct m_cycleQueue." << endl;
		return false;
	}
	m_cycleQueue->_buffer = m_buffer;
	m_cycleQueue->_size = size;
	m_cycleQueue->_in = 0;
	m_cycleQueue->_out = 0;
	m_cycleQueue->_mutex = &mutex;
	return true;
}

void CycleQueue::DSP_QueueReset() {
	if (m_cycleQueue) {
		memset(m_buffer, 0, m_cycleQueue->_size);
		m_cycleQueue->_in = 0;
		m_cycleQueue->_out = 0;
	}
}

DP_Bool CycleQueue::DSP_QueueIsEmpty() {
	pthread_mutex_lock(m_cycleQueue->_mutex);
	DP_Bool ret = queueIsEmpty();
	pthread_mutex_unlock(m_cycleQueue->_mutex);
	return ret;

}
DP_Bool CycleQueue::DSP_QueueIsFull() {
	pthread_mutex_lock(m_cycleQueue->_mutex);
	DP_Bool ret = queueIsFull();
	pthread_mutex_unlock(m_cycleQueue->_mutex);
	return ret;
}
DP_U32 CycleQueue::DSP_QueueDataLen() {
	pthread_mutex_lock(m_cycleQueue->_mutex);
	DP_U32 ret = queueDataLen();
	pthread_mutex_unlock(m_cycleQueue->_mutex);
	return ret;
}

DP_U32 CycleQueue::DSP_QueueSpaceLeft() {
	pthread_mutex_lock(m_cycleQueue->_mutex);
	DP_U32 ret = queueSpaceLeft();
	pthread_mutex_unlock(m_cycleQueue->_mutex);
	return ret;
}


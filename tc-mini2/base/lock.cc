/*================================================================
*   Copyright (C) 2014 All rights reserved.
*
*   文件名称：Lock.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年09月10日
*   描    述：
*
#include "lock.h"
================================================================*/

#include "lock.h"

CLock::CLock() {
#ifdef _WIN32
    InitializeCriticalSection(&m_critical_section);
#else
    pthread_mutex_init(&lock_, NULL);
#endif
}

CLock::~CLock() {
#ifdef _WIN32
    DeleteCriticalSection(&m_critical_section);
#else
    pthread_mutex_destroy(&lock_);
#endif
}

void CLock::lock() {
#ifdef _WIN32
    EnterCriticalSection(&m_critical_section);
#else
    pthread_mutex_lock(&lock_);
#endif
}

void CLock::unlock() {
#ifdef _WIN32
    LeaveCriticalSection(&m_critical_section);
#else
    pthread_mutex_unlock(&lock_);
#endif
}

#ifndef _WIN32
bool CLock::try_lock() { return pthread_mutex_trylock(&lock_) == 0; }
#endif

#ifndef _WIN32
CRWLock::CRWLock() { pthread_rwlock_init(&lock_, NULL); }

CRWLock::~CRWLock() { pthread_rwlock_destroy(&lock_); }

void CRWLock::rlock() { pthread_rwlock_rdlock(&lock_); }

void CRWLock::wlock() { pthread_rwlock_wrlock(&lock_); }

void CRWLock::unlock() { pthread_rwlock_unlock(&lock_); }

bool CRWLock::try_rlock() { return pthread_rwlock_tryrdlock(&lock_) == 0; }

bool CRWLock::try_wlock() { return pthread_rwlock_trywrlock(&lock_) == 0; }

CAutoRWLock::CAutoRWLock(CRWLock *pLock, bool bRLock) {
    lock_ = pLock;
    if (NULL != lock_) {
        if (bRLock) {
            lock_->rlock();
        } else {
            lock_->wlock();
        }
    }
}

CAutoRWLock::~CAutoRWLock() {
    if (NULL != lock_) {
        lock_->unlock();
    }
}
#endif

CAutoLock::CAutoLock(CLock *pLock) {
    m_pLock = pLock;
    if (NULL != m_pLock)
        m_pLock->lock();
}

CAutoLock::~CAutoLock() {
    if (NULL != m_pLock)
        m_pLock->unlock();
}

// ---------------------------------------------------------------------------
// String Library - v0.0.1 - 07/10/2020
//
// AUTHOR/LICENSE:
// Created by Imran Peerbhai -- ipeerbhai@aol.com
// Copyright 2020 License:
// Forks and derivitive works are NOT permitted without
// permission. Permission is only granted to use as-is for private and
// non-commercial use by natural persons or non-profit entities.
// For-profit entities require a license.
//
// CONTRIBUTIONS:
// If you wish to contribute, make changes, or enhancements,
// please create a pull request.
//
// DISCLAIMER:
// This software is furnished "as is", without technical support, and with no
// warranty, express or implied, as to its usefulness for any purpose.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Concept of Operations:
//  The teensy is really sensitive to new/delete/malloc issues present in Ardruino.
//  It takes very few new/delete or malloc/dalloc calls to crash the Teensy 4.1.
//  EasyString provides uses the same interface as String,
//  But does so with a series of fixed-length buffers.


#include <Arduino.h>
#ifndef EASYSTRING_ONCE
#define EASYSTRING_ONCE

#define EASY_SUBSTRINGS_LIMIT 7
#define EASY_BUFFER_SIZE 26

class EasyString{
    public:
        EasyString();
        EasyString(char *_item);
        char *substring(int start, int end);
        char *substring(int start);
        void Clear();
        void Append(const char *appendThis);
        void Append(String appendThis);
        int toInt();
        float toFloat();
        String toString();
        char *toCharArray();
        EasyString operator+=(const char *appendThis);
        EasyString operator+(const char *appendThis);
        EasyString operator+=(String appendThis);
        EasyString operator+(String appendThis);
        EasyString operator+=(EasyString appendThis);
        uint8_t *GetClearedBuffer();
        char *Get();
    private:
        char m_charBuffer[EASY_BUFFER_SIZE];
        char m_substrings[EASY_SUBSTRINGS_LIMIT][EASY_BUFFER_SIZE];
        String m_asString;
        int m_lastSubstring;
        int m_lastIndex;
};

#endif
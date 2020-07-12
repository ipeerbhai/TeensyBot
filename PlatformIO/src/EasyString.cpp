#include "EasyString.h"

EasyString::EasyString()
{
    Clear();
}

EasyString::EasyString(char *_item)
{
    Clear();
    for (int i = 0; _item[i] != '\0'; i++)
    {
        m_charBuffer[i] = _item[i];
        m_lastIndex = i;
    }
}

char *EasyString::substring(int start)
{
    return (substring(start, strlen(m_charBuffer)));
}

char *EasyString::substring(int start, int end)
{
    // clear the substring
    int retIndex = m_lastSubstring;
    for (int i = 0; i < EASY_BUFFER_SIZE; i++)
    {
        m_substrings[m_lastSubstring][i] = '\0';
    }

    for (int i = start; m_charBuffer[i] != '\0'; i++)
    {
        if (i == end)
        {
            break; // we're done
        }

        m_substrings[m_lastSubstring][i] = m_charBuffer[i];
    }

    if (m_lastSubstring == EASY_BUFFER_SIZE)
    {
        m_lastSubstring = 0;
    }
    else
    {
        m_lastSubstring++;
    }
    return (m_substrings[retIndex]);
}
uint8_t *EasyString::GetClearedBuffer()
{
    Clear();
    return ((uint8_t *)m_charBuffer);
}

char *EasyString::Get()
{
    return (m_charBuffer);
}

void EasyString::Clear()
{
    for (int i = 0; i < EASY_BUFFER_SIZE; i++)
    {
        m_charBuffer[i] = '\0';
    }
    m_lastIndex = 0;
}

void EasyString::Append(const char *appendThis)
{
    Serial.print("Appending");
    Serial.println(appendThis);
    Serial.flush();
    int len = strlen(appendThis);
    Serial.println("Appending n characters:");
    Serial.println(len);
    Serial.println("::");
    Serial.flush();
    int offsetIndex = m_lastIndex;
    for (int i = 0; i <= len; i++)
    {
        if (offsetIndex >= EASY_BUFFER_SIZE - 1)
        {
            return; // can't add, we have no buffer room left, so just truncate here.
        }
        m_charBuffer[offsetIndex] = appendThis[i];
        offsetIndex++;
    }
    m_lastIndex = offsetIndex + 1;

    Serial.print("Append complete.  String now contains:");
    Serial.println(Get());
    Serial.println("Really");
    
    Serial.flush();
}

void EasyString::Append(String appendThis)
{
    char buffer[EASY_BUFFER_SIZE];
    appendThis.toCharArray(buffer, EASY_BUFFER_SIZE);
    Append(buffer);
}

int EasyString::toInt()
{
    // left to right conversion, stopping at first non-numeric.
    int retVal;
    retVal = atoi(m_charBuffer);
    return(retVal);
}

float EasyString::toFloat()
{
    float retVal;
    retVal = atof(m_charBuffer);
    return(retVal);
}

String EasyString::toString()
{
    m_asString = String(m_charBuffer);
    return(m_asString);
}

EasyString EasyString::operator+=(String appendThis)
{
    char buffer[EASY_BUFFER_SIZE];
    appendThis.toCharArray(buffer, EASY_BUFFER_SIZE);
    Append(buffer);
    return(*this);
}

EasyString EasyString::operator+=(const char*appendThis)
{
    Append(appendThis);
    return(*this);
}

EasyString EasyString::operator+=(EasyString appendThis)
{
    Append(appendThis.Get());
    return(*this);
}

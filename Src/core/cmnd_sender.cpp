#include "core/cmnd_sender.h"
#include <cstdio>



bool cmnd_sender::send_trigger(const char* path)
{
    if (!begin_frame(path)) return false;
    return end_frame();
}


bool cmnd_sender::send_int(
    const char* path,
    const int32_t* values,
    uint16_t count)
{
    if (!begin_frame(path)) return false;

    for (uint16_t i = 0; i < count; ++i)
    {
        if (i && !push_char(',')) return false;
        if (!push_int(values[i])) return false;
    }

    return end_frame();
}
bool cmnd_sender::send_float(
    const char* path,
    const float* values,
    uint16_t count)
{
    if (!begin_frame(path)) return false;

    for (uint16_t i = 0; i < count; ++i)
    {
        if (i && !push_char(',')) return false;
        if (!push_float(values[i])) return false;
    }

    return end_frame();
}
bool cmnd_sender::send_string(
    const char* path,
    const char* const* values,
    uint16_t count)
{
    if (!begin_frame(path)) return false;

    for (uint16_t i = 0; i < count; ++i)
    {
        if (i && !push_char(',')) return false;
        if (!push_string(values[i])) return false;
    }

    return end_frame();
}

bool cmnd_sender::push_char(char c)
{
	if (!tx_queue.push(static_cast<uint8_t>(c)))
		return false;

	notify_tx_ready();   // HER BYTE SONRASI
	return true;
}
bool cmnd_sender::push_string(const char* s)
{
    while (*s)
    {
        if (!push_char(*s++))
            return false;
    }
    return true;
}
bool cmnd_sender::push_int(int32_t v)
{
	if (v == INT32_MIN)
		return push_string("-2147483648");

    char buf[12]; // -2147483648 + '\0'
    int i = 0;

    if (v == 0)
        return push_char('0');

    bool negative = false;
    if (v < 0)
    {
        negative = true;
        v = -v;
    }

    while (v > 0)
    {
        buf[i++] = '0' + (v % 10);
        v /= 10;
    }

    if (negative)
        buf[i++] = '-';

    while (i--)
    {
        if (!push_char(buf[i]))
            return false;
    }

    return true;
}

bool cmnd_sender::push_float(float v)
{
    if (v < 0.0f)
    {
        if (!push_char('-'))
            return false;
        v = -v;
    }

    int32_t int_part = (int32_t)v;
    float frac = v - (float)int_part;

    if (!push_int(int_part))
        return false;

    if (!push_char('.'))
        return false;

    // 3 decimal
    int32_t frac_part = (int32_t)(frac * 1000.0f + 0.5f);

    if (frac_part >= 1000)
    {
    	frac_part = 0;
        int_part++;
    }

    // leading zero padding
    if (frac_part < 100) {
        if (!push_char('0')) return false;
    }
    if (frac_part < 10) {
        if (!push_char('0')) return false;
    }

    return push_int(frac_part);
}
bool cmnd_sender::begin_frame(const char* path)
{
    // {p:<path>:d:
    if (!push_char('{')) return false;
    if (!push_char('p')) return false;
    if (!push_char(':')) return false;
    if (!push_string(path)) return false;
    if (!push_char(':')) return false;
    if (!push_char('d')) return false;
    if (!push_char(':')) return false;
    return true;
}

bool cmnd_sender::end_frame()
{
	if (!push_char('}'))
		return false;

	return true;
}

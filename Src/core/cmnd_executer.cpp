#include "core/cmnd_executer.h"



cmnd_executer::cmnd_executer(
    ring_buffer<cmnd_frame>& frame_buffer,
    const path_entry* table,
    uint16_t table_size)
    : frame_queue(frame_buffer),
      path_table(table),
      path_count(table_size)
{
}


static data_type detect_type(const char* data)
{
    bool has_dot = false;

    for (const char* p = data; *p; p++)
    {
        if (*p == '.')
            has_dot = true;
        else if ((*p < '0' || *p > '9') && *p != '-' && *p != ',')
            return data_type::STRING;
    }

    return has_dot ? data_type::FLOAT : data_type::INT;
}
static uint16_t count_csv(const char* data)
{
    uint16_t count = 1;

    for (const char* p = data; *p; p++)
    {
        if (*p == ',')
            count++;
    }

    return count;
}
static uint16_t parse_int_csv(const char* data, int32_t* out)
{
    uint16_t count = 0;

    while (*data && count < MAX_CSV_ITEMS)
    {
        out[count++] = atoi(data);

        while (*data && *data != ',')
            data++;

        if (*data == ',')
            data++;
    }

    return count;
}
static uint16_t parse_float_csv(const char* data, float* out)
{
    uint16_t count = 0;

    while (*data && count < MAX_CSV_ITEMS)
    {
        out[count++] = strtof(data, nullptr);

        while (*data && *data != ',')
            data++;

        if (*data == ',')
            data++;
    }

    return count;
}
static uint16_t parse_string_csv(char* data, char** out)
{
    uint16_t count = 0;

    out[count++] = data;

    while (*data && count < MAX_CSV_ITEMS)
    {
        if (*data == ',')
        {
            *data = '\0';
            out[count++] = data + 1;
        }
        data++;
    }

    return count;
}
void cmnd_executer::poll()
{
    cmnd_frame frame;

    if (!frame_queue.pop(frame))
        return;

    for (uint16_t i = 0; i < path_count; i++)
    {
        if (strcmp(frame.path, path_table[i].path) != 0)
            continue;

        // 1.If no data
        if (frame.data == nullptr || frame.data_len == 0)
        {
            path_table[i].handler(
                data_type::NONE,
                nullptr,
                0
            );
            return;
        }

        // 2.Dedect the data type.
        data_type type = detect_type(frame.data);

        if (type != path_table[i].expected_type)
        {
            // mismatch → drop command
            return;
        }

        // 3.CSV parse + dispatch
        switch (type)
        {
			case data_type::INT:
			{
				int32_t values[MAX_CSV_ITEMS];
				uint16_t count = parse_int_csv(frame.data, values);

				path_table[i].handler(
					data_type::INT,
					values,
					count
				);
				break;
			}

			case data_type::FLOAT:
			{
				float values[MAX_CSV_ITEMS];
				uint16_t count = parse_float_csv(frame.data, values);

				path_table[i].handler(
					data_type::FLOAT,
					values,
					count
				);
				break;
			}

			case data_type::STRING:
			{
				char* values[MAX_CSV_ITEMS];
				uint16_t count = parse_string_csv(
					(char*)frame.data,
					values
				);

				path_table[i].handler(
					data_type::STRING,
					values,
					count
				);
				break;
			}

			default:
				break;
        }

        return;
    }
}




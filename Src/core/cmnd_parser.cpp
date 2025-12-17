#include "core/cmnd_parser.h"

cmnd_parser::cmnd_parser(ring_buffer<uint8_t>& rx_buffer,
                         ring_buffer<cmnd_frame>& frame_buffer,
                         char* work_buffer,
                         uint16_t work_buffer_size)
    : rx_queue(rx_buffer),
      frame_queue(frame_buffer),
      workBuffer(work_buffer),
      work_buf_size(work_buffer_size),
      idx(0),
      path_ptr(nullptr),
      path_len(0),
      data_ptr(nullptr),
      data_len(0),
      state(state_t::WAIT_START)
{
}

void cmnd_parser::reset()
{
    state    = state_t::WAIT_START;
    idx      = 0;
    path_ptr = nullptr;
    data_ptr = nullptr;
    path_len = 0;
    data_len = 0;
}

void cmnd_parser::poll()
{
    uint8_t ch;

    while (rx_queue.pop(ch))
    {
        // Overflow guard
    	if (idx >= work_buf_size)
    	{
    	    reset();
    	    state = state_t::ERROR;
    	    continue;   // <<<<< ÇOK ÖNEMLİ
    	}

        switch (state)
        {
        case state_t::WAIT_START:
            if (ch == '{')
            {
                reset();
                state = state_t::WAIT_P;
            }
            break;

        case state_t::WAIT_P:
            state = (ch == 'p') ? state_t::WAIT_P_COLON : state_t::ERROR;
            break;

        case state_t::WAIT_P_COLON:
            state = (ch == ':') ? state_t::READ_PATH : state_t::ERROR;
            if (state == state_t::READ_PATH)
                path_ptr = &workBuffer[idx];
            break;

        case state_t::READ_PATH:
            if (ch == ':')
            {
                workBuffer[idx++] = '\0';
                path_len = idx - 1;
                state = state_t::WAIT_D;
            }
            else
            {
                workBuffer[idx++] = ch;
            }
            break;

        case state_t::WAIT_D:
            state = (ch == 'd') ? state_t::WAIT_D_COLON : state_t::ERROR;
            break;

        case state_t::WAIT_D_COLON:
            if (ch == ':')
            {
                data_ptr = &workBuffer[idx];
                state = state_t::READ_DATA;
            }
            else
            {
                state = state_t::ERROR;
            }
            break;

        case state_t::READ_DATA:
            if (ch == '}')
            {
                workBuffer[idx++] = '\0';
                data_len = idx - (data_ptr - workBuffer) - 1;

                cmnd_frame frame {
                    path_ptr,
                    path_len,
                    data_ptr,
                    data_len
                };

                if (!frame_queue.push(frame)){
                    reset();
                    state = state_t::ERROR;
                    continue;
                }

                reset();
            }
            else
            {
                workBuffer[idx++] = ch;
            }
            break;

        case state_t::ERROR:
        	idx = 0;
            if (ch == '{')
            {
                reset();
                state = state_t::WAIT_P;
            }
            break;

        default:
            reset();
            break;
        }
    }
}

/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <httpd.h>
#include <http_log.h>
#include <http_protocol.h>
#include <util_filter.h>
#include <unistd.h>
#include <time.h>
#include <apr_strings.h>
#include <stdarg.h>


#include "tinia/trell/trell.h"
#include "mod_trell.h"

// return 0 on success & finished, -1 failure, and 1 on longpoll wanted.
int
trell_pass_reply_assert_ok( void* data,
                            unsigned char* buffer,
                            size_t offset,
                            size_t bytes,
                            int more )
{
    //trell_callback_data_t* cbd = (trell_callback_data_t*)data;
    trell_message_t* msg = (trell_message_t*)buffer;
    if( msg->m_type == TRELL_MESSAGE_OK ) {
        return MESSENGER_OK;
    }
    else {
        return -1;
    }
}


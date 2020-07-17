#ifndef _QUEUE_H_
#define _QUEUE_H_
/*
    NET2 is a threaded, event based, network IO library for SDL.
    Copyright (C) 2002 Bob Pendleton

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation; either version 2.1
    of the License, or (at your option) any later version.
    
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA

    If you do not wish to comply with the terms of the LGPL please
    contact the author as other terms are available for a fee.
    
    Bob Pendleton
    Bob@Pendleton.com
*/

//
// simple fixed length queue
//

#define QUEUETYPE(NAME, TYPE, SIZE)  \
  \
typedef struct  \
{  \
  int length;  \
  int head;  \
  int tail;  \
  TYPE buf[(SIZE)];  \
} NAME##Que;

#define QUEUEFORWARD(MODIFIER, NAME, TYPE)  \
  \
MODIFIER void Init##NAME##Que(NAME##Que *q);  \
MODIFIER int NAME##QueFull(NAME##Que *q);  \
MODIFIER int NAME##QueEmpty(NAME##Que *q);  \
MODIFIER int Enque##NAME(NAME##Que *q, TYPE *val);  \
MODIFIER int Deque##NAME(NAME##Que *q, TYPE *val);  \

#define QUEUECODE(MODIFIER, NAME, TYPE, SIZE)  \
  \
MODIFIER void Init##NAME##Que(NAME##Que *q)  \
{  \
  q->length = 0;  \
  q->head = 0;  \
  q->tail = 0;  \
}  \
  \
MODIFIER int NAME##QueFull(NAME##Que *q)  \
{  \
  return ((SIZE) == q->length);  \
}  \
  \
MODIFIER int NAME##QueEmpty(NAME##Que *q)  \
{  \
  return (0 == q->length);  \
}  \
  \
MODIFIER int Enque##NAME(NAME##Que *q, TYPE *val)  \
{  \
  if ((SIZE) == q->length)  \
  {  \
    return -1;  \
  }  \
  \
  q->length++;  \
  q->buf[q->tail] = *val;  \
  q->tail = ((q->tail + 1) % (SIZE));  \
    \
  return 0;  \
}  \
  \
MODIFIER int Deque##NAME(NAME##Que *q, TYPE *val)  \
{  \
  if (0 == q->length)  \
  {  \
    return -1;  \
  }  \
  \
  q->length--;  \
  *val = q->buf[q->head];  \
  q->head = (q->head + 1) % (SIZE);  \
  \
  return 0;  \
}

#endif

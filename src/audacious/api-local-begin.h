/*
 * api-local-begin.h
 * Copyright 2010 John Lindgren
 *
 * This file is part of Audacious.
 *
 * Audacious is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 or version 3 of the License.
 *
 * Audacious is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Audacious. If not, see <http://www.gnu.org/licenses/>.
 *
 * The Audacious team does not consider modular code linking to Audacious or
 * using our public API to be a derived work.
 */

#if ! defined AUD_API_NAME || ! defined AUD_API_SYMBOL || defined AUD_API_LOCAL_H
#error Bad usage of api-local-begin.h
#endif

#define AUD_API_LOCAL_H

#define AUD_FUNC0(t,n) t n (void);
#define AUD_FUNC1(t,n,t1,n1) t n (t1 n1);
#define AUD_FUNC2(t,n,t1,n1,t2,n2) t n (t1 n1, t2 n2);
#define AUD_FUNC3(t,n,t1,n1,t2,n2,t3,n3) t n (t1 n1, t2 n2, t3 n3);
#define AUD_FUNC4(t,n,t1,n1,t2,n2,t3,n3,t4,n4) t n (t1 n1, t2 n2, t3 n3, t4 n4);
#define AUD_FUNC5(t,n,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5) t n (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5);
#define AUD_FUNC6(t,n,t1,n1,t2,n2,t3,n3,t4,n4,t5,n5,t6,n6) t n (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6);

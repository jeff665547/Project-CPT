#pragma once
#define DERIVE_ID( T, ID ) \
static int derive_id ( const T& x ) \
{ \
    return ID; \
}

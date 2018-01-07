
NBRE_IOStream&
NBRE_IOStream::operator>>(uint16& val) 
{
    uint32 readed = 0;
    Read((uint8*)&val, sizeof(val), &readed);
    nbre_assert(readed == sizeof(val));
    return *this;
}

NBRE_IOStream&
NBRE_IOStream::operator>>(uint32& val) 
{
    uint32 readed = 0;
    Read((uint8*)&val, sizeof(val), &readed);
    nbre_assert(readed == sizeof(val));
    return *this;
}

NBRE_IOStream&
NBRE_IOStream::operator>>(int16& val) 
{
    uint32 readed = 0;
    Read((uint8*)&val, sizeof(val), &readed);
    nbre_assert(readed == sizeof(val));
    return *this;
}

NBRE_IOStream&
NBRE_IOStream::operator>>(int32& val) 
{
    uint32 readed = 0;
    Read((uint8*)&val, sizeof(val), &readed);
    nbre_assert(readed == sizeof(val));
    return *this;
}

NBRE_IOStream&
NBRE_IOStream::operator>>(float& val) 
{
    uint32 readed = 0;
    Read((uint8*)&val, sizeof(val), &readed);
    nbre_assert(readed == sizeof(val));
    return *this;
}

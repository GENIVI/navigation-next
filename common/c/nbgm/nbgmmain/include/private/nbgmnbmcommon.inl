
template<typename T>
static void
ClearMap(T& map)
{
    T::iterator i;
    T::iterator end = map.end();

    for(i = map.begin(); i!= end; ++i)
    {
        NBRE_DELETE i->second;
    }
}

inline NBGM_NBMField::~NBGM_NBMField()
{
    ClearMap<NBRE_Map<NBRE_String, NBGM_NBMAttribute*>>(mAttributes);
    ClearMap<NBRE_Map<NBRE_String, NBGM_NBMField*>>(mSubFields);
    ClearMap<NBRE_Map<NBRE_String, NBGM_NBMGroup*>>(mGroups);
}

inline NBGM_NBMGroup::~NBGM_NBMGroup()
{
    for(size_t i=0; i<mFieldArray.size(); ++i)
    {
        NBRE_DELETE mFieldArray[i];
    }
}

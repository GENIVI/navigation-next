TypeCpp(type) ::= <<
/****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     NBProtocol<type.name>.cpp
    @defgroup protocol
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "NBProtocol<type.name>.h"
<if(type.base.Abstract)>
#include "cslutil.h"
#include \<sstream\>
<endif>
using namespace protocol;

<type.name>::<type.name>()
    <if(type.BinaryData)>
    : data(NULL)
    <endif>
{
    // Constructor
    Initialize();
}

<type.name>::~<type.name>()
{
    // Destructor
    <if(type.BinaryData)>
    nsl_free(data);
    <endif>
}

<type.name>& <type.name>::operator=(const <type.name>& obj<type.name>)
{
    // Handle self-assignment:
    if(this == &obj<type.name>) return *this;

    <if(type.BinaryData)>
    if (data)
    {
        nsl_free(data);
    }
    <endif>

    Initialize();
    <if(type.BinaryData)>
    length = objBinaryData.GetLength();
    data = (byte*)nsl_malloc(length);
    nsl_memcpy(data, objBinaryData.GetData(), length);
<else>
<type.fields: { field |
<if(!field.Array)>
    <if(!field.type.BuiltIn)>
    if((obj<type.name>.Get<field.name;format="toUpperFirst">()))
    {
        <field.name> =shared_ptr\<<field.type.name>\>(new <field.type.name>(*(obj<type.name>.Get<field.name;format="toUpperFirst">())));
    \}
    <else>
    <if(!field.String)>
    <field.name> = obj<type.name>.Get<field.name;format="toUpperFirst">();
    <else>
    if((obj<type.name>.Get<field.name;format="toUpperFirst">()))
    {
        <field.name> = shared_ptr\<std::string\>(new std::string(obj<type.name>.Get<field.name;format="toUpperFirst">()->c_str()));
    \}
    <endif>
    <endif>
<else>
    <if(!field.type.BuiltIn)>
        //Copy Array(not built-in type)
        for(uint32 i=0;i\<obj<type.name>.Get<field.name;format="toUpperFirst">Array()->size();i++)
        {
            if((obj<type.name>.Get<field.name;format="toUpperFirst">Array()->at(i)))
            {
                <field.name>->push_back(shared_ptr\<<field.type.name>\>(new <field.type.name>(*(obj<type.name>.Get<field.name;format="toUpperFirst">Array()->at(i)))));
            \}
        \}
    <else>
    <if(!field.String)>
        //Copy Array(built-in type)
        for(uint32 i=0;i\<obj<type.name>.Get<field.name;format="toUpperFirst">Array()->size();i++)
        {
            <field.name>->push_back(obj<type.name>.Get<field.name;format="toUpperFirst">Array()->at(i));
        \}
    <else>
        //Copy Array(String)
         for(uint32 i=0;i\<obj<type.name>.Get<field.name;format="toUpperFirst">Array()->size();i++)
        {
            if((objAddress.Get<field.name;format="toUpperFirst">Array()->at(i)))
            {
                //shared_ptr\<std::string\> stringPtr = shared_ptr\<std::string\>(new std::string(objAddress.Get<field.name;format="toUpperFirst">Array()->at(i)->c_str()));
                <field.name>->push_back(shared_ptr\<std::string\>(new std::string(*(objAddress.Get<field.name;format="toUpperFirst">Array()->at(i)))));
            \}
        \}
    <endif>
    <endif>
<endif>
}>
<endif>
    return *this;
}

<type.name>::<type.name>(const <type.name>& obj<type.name>)
{

    Initialize();
    //Copy Constructor
<if (type.BinaryData)>
    length = objBinaryData.GetLength();
    data = (byte*)nsl_malloc(length);
    nsl_memcpy(data, objBinaryData.GetData(), length);
<else>
<type.fields: { field |
<if(!field.Array)>
    <if(!field.type.BuiltIn)>
    if((obj<type.name>.Get<field.name;format="toUpperFirst">()))
    {
        <field.name> =shared_ptr\<<field.type.name>\>(new <field.type.name>(*(obj<type.name>.Get<field.name;format="toUpperFirst">())));
    \}
    <else>
    <if(!field.String)>
    <field.name> = obj<type.name>.Get<field.name;format="toUpperFirst">();
    <else>
    if((obj<type.name>.Get<field.name;format="toUpperFirst">()))
    {
        <field.name> = shared_ptr\<std::string\>(new std::string(obj<type.name>.Get<field.name;format="toUpperFirst">()->c_str()));
    \}
    <endif>
    <endif>
<else>
    <if(!field.type.BuiltIn)>
        //Copy Array(not built-in type)
        for(uint32 i=0;i\<obj<type.name>.Get<field.name;format="toUpperFirst">Array()->size();i++)
        {
            if((obj<type.name>.Get<field.name;format="toUpperFirst">Array()->at(i)))
            {
                <field.name>->push_back(shared_ptr\<<field.type.name>\>(new <field.type.name>(*(obj<type.name>.Get<field.name;format="toUpperFirst">Array()->at(i)))));
            \}
        \}
    <else>
    <if(!field.String)>
        //Copy Array(built-in type)
        for(uint32 i=0;i\<obj<type.name>.Get<field.name;format="toUpperFirst">Array()->size();i++)
        {
            <field.name>->push_back(obj<type.name>.Get<field.name;format="toUpperFirst">Array()->at(i));
        \}
    <else>
        //Copy Array(String)
         for(uint32 i=0;i\<obj<type.name>.Get<field.name;format="toUpperFirst">Array()->size();i++)
        {
            if((objAddress.Get<field.name;format="toUpperFirst">Array()->at(i)))
            {
            //shared_ptr\<std::string\> stringPtr = shared_ptr\<std::string\>(new std::string(objAddress.Get<field.name;format="toUpperFirst">Array()->at(i)->c_str()));
            <field.name>->push_back(shared_ptr\<std::string\>(new std::string(*(objAddress.Get<field.name;format="toUpperFirst">Array()->at(i)))));
            \}
        \}
    <endif>
    <endif>
<endif>
}>
<endif>
}

void <type.name>::Initialize()
{
    //For non built-in and non-array type there is AutoSharedPointer to that type
    //That pointer will be initialzed with a reference count of 0;
    <type.fields: { field |
    <if(!field.Array)>
     <if(field.type.BuiltIn)>
       <if(!field.String)>
<field.name> = 0;
      <endif>
    <endif>
    <else>
        <if(!field.type.BuiltIn)>
        //std::vector of AutoSharedPointers for not built types
         <field.name> = shared_ptr\<std::vector\<shared_ptr\<<field.type.name>\> \> \>(new std::vector\<shared_ptr\<<field.type.name>\> \>());
        <else>
        <if(!field.String)>
        //std::vector of AutoSharedPointers for UtfString types
        <field.name> = shared_ptr\<std::vector\<<field.type.name> \> \>(new std::vector\<<field.type.name> \>());
        <else>
        //std::vector of AutoSharedPointers for built in types
         <field.name> = shared_ptr\<std::vector\<shared_ptr\<std::string\> \> \>(new std::vector\<shared_ptr\<std::string\> \>());
        <endif>
        <endif>
    <endif>
    }>
}

<if(type.base.Abstract)>
<if(!type.BinaryData)>
shared_ptr\<std::string\> <type.name>::ToXML() const
{
    shared_ptr\<std::string\> retval(new std::string("\<<type.wireName> attribute=\"false\"\>"));

<type.fields: { field |
<if(!field.Array)>
<if(!field.type.BuiltIn)>
<if ((field.type.BinaryData) && (field.Simple))>
    if (NULL != this->Get<field.name;format="toUpperFirst">())
    {
        retval->append("\<<field.wireName> attribute=\"true\" type=\"binary\">");
        std::string result;
        char* encodedSerializedData = NULL;
        NB_Error err = encode_base64((const char*)this->Get<field.name;format="toUpperFirst">()->GetData(), this->Get<field.name;format="toUpperFirst">()->GetLength(), &encodedSerializedData);
        if (err == NE_OK)
        {
            result = std::string(encodedSerializedData);
            nsl_free(encodedSerializedData);
        \}
        retval->append(result);
        retval->append("\</<field.wireName>\>");
    \}
<elseif (!(field.type.BinaryData) && !(field.Simple))>
    if (this->Get<field.name;format="toUpperFirst">())
    {
        retval->append(this->Get<field.name;format="toUpperFirst">()->ToXML()->c_str());
    \}
<endif>
<else>
<if(!field.type.Bool)>
<if (field.Simple)>
<if(field.String)>
    retval->append("\<<field.wireName> attribute=\"true\" type=\"string\">");
    if(this->Get<field.name;format="toUpperFirst">() != NULL)
    {
        retval->append(this->Get<field.name;format="toUpperFirst">()->c_str());
    \}
    retval->append("\</<field.wireName>\>");
<else>
    retval->append("\<<field.wireName> attribute=\"true\" type=\"<field.type.name>\">");
    std::ostringstream oss_<field.name>;
    oss_<field.name> \<\< this->Get<field.name;format="toUpperFirst">();
    retval->append(oss_<field.name>.str());
    retval->append("\</<field.wireName>\>");
<endif>
<endif>
<elseif (!(field.ForVerification))>
    if (this->Get<field.name;format="toUpperFirst">())
    {
        retval->append("\<<field.wireName> attribute=\"false\">");
        retval->append("\</<field.wireName>\>");
    \}
<endif>
<endif>
<else>
<if(!field.type.BuiltIn)>
    for (uint32 i = 0; i\<this->Get<field.name;format="toUpperFirst">Array()->size(); i++)
    {
        retval->append(this->Get<field.name;format="toUpperFirst">Array()->at(i)->ToXML()->c_str());
    \}
<else>
    for (uint32 i = 0; i\<this->Get<field.name;format="toUpperFirst">Array()->size(); i++)
    {
        std::ostringstream oss;

        <if(!field.type.Bool)>
        <if (field.Simple)>
        <if(field.String)>
        retval->append("\<<field.wireName> attribute=\"true\" type=\"string\">");
        retval->append(this->Get<field.name;format="toUpperFirst">()->c_str());
        <else>

        retval->append("\<<field.wireName> attribute=\"true\" type=\"<field.type.name>\">");
        oss \<\< this->Get<field.name;format="toUpperFirst">();
        retval->append(oss.str());
        <endif>
        retval->append("\</<field.wireName>\>");
        <endif>
        <elseif (!(field.ForVerification))>
        retval->append("\<<field.wireName> attribute=\"false\">")
        retval->append("\</<field.wireName>\>");
        <endif>
   \}
<endif>
<endif>}>
    retval->append("\</<type.wireName>\>");

    return retval;
}

<else>
shared_ptr\<std::string\> <type.name>::ToXML() const
{
    shared_ptr\<std::string\> retval(new std::string("\<<type.wireName> attribute=\"false\"\>"));

    retval->append("\</<type.wireName>\>");
    return retval;
}
<endif>
<endif>
// Setter methods
<if(!type.BinaryData)>
<type.fields: { field |
<if(!field.Array)>
void <type.name>::Set<field.name;format="toUpperFirst"> (<if(!field.type.BuiltIn)>shared_ptr\<<field.type.name;format="toUpperFirst">\> <else><field.type.name><endif> <field.name>)
{
    this-><field.name> = <field.name>;
\}
<! <else> !>
<! void <type.name>::Set<field.name;format="toUpperFirst">Array(shared_ptr\<std::vector\<<if(!field.type.BuiltIn)>shared_ptr\<<field.type.name;format="toUpperFirst">\> <else><field.type.name><endif> \> \> <field.name>)
{
    this-><field.name> = <field.name>;
\} !>
<endif>
}>
<else>
void <type.name>::SetData(byte* data, unsigned int length)
{
    this->length = length;
    this->data = (byte*)nsl_malloc(length);
    nsl_memcpy(this->data, data, length);
}
<endif>

// Getter methods
<type.fields: { field |
<if(!field.Array)>
<if(!field.type.BuiltIn)>shared_ptr\<<field.type.name;format="toUpperFirst">\><else><if(!field.type.BuiltIn)><field.type.name;format="toUpperFirst"> <else><field.type.name><endif><endif> <type.name>::Get<field.name;format="toUpperFirst">() const
{
    return <field.name>;
\}
<else>
shared_ptr\<std::vector\<<if(!field.type.BuiltIn)>shared_ptr\<<field.type.name;format="toUpperFirst">\> <else><field.type.name><endif> \> \> <type.name>::Get<field.name;format="toUpperFirst">Array() const
{
    return <field.name>;
\}
<endif>
}>

/*! @} */
>>

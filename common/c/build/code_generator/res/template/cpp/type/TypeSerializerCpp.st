TypeSerializerCpp(type) ::= <<
/****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     NBProtocol<type.name>Serializer.cpp
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

#include "NBProtocol<type.name>Serializer.h"
<type.fields: { field |
<if(!field.type.BuiltIn)>#include "NBProtocol<field.type.name;format="toUpperFirst">Serializer.h"
<endif>}>
using namespace protocol;

<type.name>Serializer::<type.name>Serializer()
{
//Constructor
}
<type.name>Serializer::~<type.name>Serializer()
{
//Destructor
}
<if(!type.BinaryData)>
TpsElementPtr <type.name>Serializer::serialize(shared_ptr\<<type.name>\> obj<type.name>)
{
    TpsElementPtr retval(new TpsElement(CHAR_PTR_TO_UTF_STRING_PTR("<type.wireName;format="toLower">")));

    if (!obj<type.name> || !retval->IsValid())
    {
        goto errexit;
    }

<type.fields: { field |
<if(!field.Array)>
<if(!field.type.BuiltIn)>
<if ((field.type.BinaryData) && (field.Simple))>
<if (field.Verification)>
  if (obj<type.name>->Get<field.verificationItem;format="toUpperFirst">())
  {
       if (NE_OK != retval->SetAttribute(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName>"), (char*)obj<type.name>->Get<field.name;format="toUpperFirst">()->GetData(),obj<type.name>->Get<field.name;format="toUpperFirst">()->GetLength()))
       {
             goto errexit;
       \}
  \}
<else>
  if (NULL != obj<type.name>->Get<field.name;format="toUpperFirst">())
  {
      if (NE_OK != retval->SetAttribute(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName>"), (char*)obj<type.name>->Get<field.name;format="toUpperFirst">()->GetData(),obj<type.name>->Get<field.name;format="toUpperFirst">()->GetLength()))
      {
        goto errexit;
      \}
  \}
  else
  {
      if (NE_OK != retval->SetAttribute(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName>"), EMPTY_STRING, 0))
      {
            goto errexit;
      \}
  \}
<endif>
<elseif (!(field.type.BinaryData) && !(field.Simple))>
<if (field.Optional)>
   if (obj<type.name>->Get<field.name;format="toUpperFirst">())
<endif>
    if (NE_OK != retval->Attach(<field.type.name;format="toUpperFirst">Serializer::serialize(obj<type.name>->Get<field.name;format="toUpperFirst">())))
    {
        goto errexit;
    \}
<endif>
    <else>
    <if(!field.type.Bool)>
    <if (field.Simple)>
  <if (field.Verification)>
  if (obj<type.name>->Get<field.verificationItem;format="toUpperFirst">())
  <endif>
          if (NE_OK != retval->SetAttribute<if(!field.String)><field.type.name;format="spaceToCamelCase"><else>String<endif>(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName>"),     obj<type.name>->Get<field.name;format="toUpperFirst">()))          goto errexit;

    <else>
        if(obj<type.name>->Get<field.name;format="toUpperFirst">())
        {
        if(NE_OK != retval->Attach(TpsElementPtr(new TpsElement(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName>")))))
        {
        goto errexit;
        \}
        \}

    <endif>
    <elseif (!(field.ForVerification))>
    if(obj<type.name>->Get<field.name;format="toUpperFirst">())
    {
        if(NE_OK != retval->Attach(TpsElementPtr(new TpsElement(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName;format="toLower">")))))
        {
            goto errexit;
        \}
    \}
    <endif>
    <endif>
<else>
    <if(!field.type.BuiltIn)>
    for (uint32 i = 0; i\<obj<type.name>->Get<field.name;format="toUpperFirst">Array()->size(); i++)
    {
        if (NE_OK != retval->Attach(<field.type.name;format="toUpperFirst">Serializer::serialize(obj<type.name>->Get<field.name;format="toUpperFirst">Array()->at(i))))
        {
            goto errexit;
        \}
    \}
    <else>
        for (uint32 i = 0; i\<obj<type.name>->Get<field.name;format="toUpperFirst">Array()->size(); i++)
        {
            if (NE_OK != retval->SetAttribute<if(!field.String)><field.type.name;format="spaceToCamelCase"><else>String<endif>(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName;format="toLower">"), obj<type.name>->Get<field.name;format="toUpperFirst">Array()->at(i)))
            {
                goto errexit;
            \}
        \}
    <endif>
<endif>}>

    return retval;
errexit:
     retval.reset();
     return retval;
}

shared_ptr\<<type.name>\> <type.name>Serializer::deserialize(TpsElementPtr tp)
{
<if(!type.Abstract)>
    shared_ptr\<<type.name>\> retval(new <type.name>());

    if (!tp || !tp->IsValid() || !retval)
    {
        goto errexit;
    }
    else
    {
<type.fields: { field |
<if(!field.Array)>
    <if(!field.type.BuiltIn)>
        <if(field.type.BinaryData)>
        <if(field.Simple)>

          {  char*    data = NULL;
          uint32    size = 0;

          tp->GetAttribute(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName;format="toLower">"), &data,&size);
          shared_ptr\<BinaryData\> bd(new BinaryData);
          if (NULL == bd)
          {
            goto errexit;
          \}
          bd->SetData((byte*)data, size);
          retval->Set<field.name;format="toUpperFirst">(bd);
        \}
        <else>
          retval->Set<field.name;format="toUpperFirst">(<field.type.name;format="toUpperFirst">Serializer::deserialize(tp->GetChild(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName;format="toLower">"))));
        <endif>
        <else>
    retval->Set<field.name;format="toUpperFirst">(<field.type.name;format="toUpperFirst">Serializer::deserialize(tp->GetChild(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName;format="toLower">"))));
        <endif>
    <else>
        <if(!field.type.Bool)>
        retval->Set<field.name;format="toUpperFirst">(tp->GetAttributeAs<if(!field.String)><field.type.name;format="spaceToCamelCase"><else>String<endif>(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName;format="toLower">")));
        <else>
    (tp->GetChild(CHAR_PTR_TO_UTF_STRING_PTR("<field.wireName;format="toLower">"))) ? retval->Set<field.name;format="toUpperFirst">(TRUE) : retval->Set<field.name;format="toUpperFirst">(FALSE);
        <endif>
    <endif>
<endif>}>

    //Deserialize arrays
    <type.fields: { field |
    <if(field.Array)>
    {
        int iter = 0;
        TpsElementPtr ce;
        while((ce = tp->GetNextChild(&iter)) != NULL && ce->IsValid())
        {
        <if(!field.type.BuiltIn)>
            if(0 == nsl_strcmp(ce->GetElementName()->c_str(), "<field.wireName;format="toLower">"))
            {
                //Array of not built in type
                retval->Get<field.name;format="toUpperFirst">Array()->push_back(<field.type.name>Serializer::deserialize(ce));
            \}
        <else>
            if(nsl_strcmp(ce->GetElementName()->c_str(), "<field.wireName;format="toLower">"))
            {
            <if(!field.String)>
                //Array of Buit-in Type
                retval->Get<field.name;format="toUpperFirst">Array()->push_back((ce->GetAttributeAs<field.type.name;format="spaceToCamelCase">(CHAR_PTR_TO_UTF_STRING_PTR("<field.name>"))));
            <else>
                //Array of String
                retval->Get<field.name;format="toUpperFirst">Array()->push_back(shared_ptr\<std::string\>( new std::string(*(ce->GetAttributeAsString(CHAR_PTR_TO_UTF_STRING_PTR("<field.name>"))))));
            <endif>
            \}
        <endif>
        \}
    \}
    <endif>}>
    return retval;
    }
<else>
    shared_ptr\<<type.name>\> retval;
    return retval;
<endif>
<if(!type.Abstract)>
errexit:
     retval.reset();
     return retval;
<endif>
}

<else>
TpsElementPtr <type.name>Serializer::serialize(shared_ptr\<<type.name>\> obj<type.name>)
{
    TpsElementPtr retval(new TpsElement(CHAR_PTR_TO_UTF_STRING_PTR("<type.wireName;format="toLower">")));

    if (!obj<type.name> || !retval->IsValid())
    {
        goto errexit;
    \}

     if (NE_OK != retval->SetAttribute(CHAR_PTR_TO_UTF_STRING_PTR("<type.wireName;format="toLower">"), (char*)obj<type.name>->GetData(), obj<type.name>->GetLength()))
     {
         goto errexit;
     \}

    return retval;
errexit:
     retval.reset();
     return retval;
\}
shared_ptr\<<type.name>\> <type.name>Serializer::deserialize(TpsElementPtr tp)
{
<if(!type.Abstract)>
    shared_ptr\<<type.name>\> retval = shared_ptr\<<type.name>\>(new <type.name>());
    TpsElementPtr ce;

    char* data = NULL;
    uint32 length= 0;


    if (!tp || !tp->IsValid() || !retval)
    {
        goto errexit;
    \}

    tp->GetAttribute(CHAR_PTR_TO_UTF_STRING_PTR("<type.wireName;format="toLower">"),&data,&length);

    retval->SetData((byte*)data,length);
<else>
    shared_ptr\<<type.name>\> retval;
<endif>
    return retval;

<if(!type.Abstract)>
errexit:
     retval.reset();
     return retval;
<endif>
\}
<endif>

/*! @} */
>>

TypeH(type) ::= <<
/****************************************************************************/
/************       THIS IS GENERATED CODE. DO NOT MODIFY        ************/
/****************************************************************************/

/*!--------------------------------------------------------------------------
    @file     NBProtocol<type.name>.h
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

#ifndef <Define(type, "")>
#define <Define(type, "")>

extern "C" {
//#include "vec.h"
//#include "paltypes.h"
}
#include "nbexp.h"
#include "smartpointer.h"
#include \<vector\>
#include "base.h"
<if(type.base.Abstract)>
#include "NBProtocol<type.base.name;format="toUpperFirst">.h"
#include \<string\>
<elseif(type.Abstract)>
#include \<string\>
<endif>

<type.fields: { field |
<if(!field.type.BuiltIn)>#include "NBProtocol<field.type.name;format="toUpperFirst">.h"
<endif>}>

namespace  protocol {
<type.fields: { field |
<if(!field.type.BuiltIn)>class <field.type.name;format="toUpperFirst">;
<endif>}>
<if(type.base.Abstract)>
class <type.base.name;format="toUpperFirst">;
<endif>
/*! Class <type.name>
    <type.description>
*/
class <type.name> : public <if(type.base.Abstract)><type.base.name><else>Base<endif>
{
public:
    /*! Constructor
    */
    <type.name>();

    /*! Destructor
    */
    ~<type.name>();

    /*! Copy Constructor.
    */
    <type.name>(const <type.name>& obj<type.name>);

    /*! Equal to operator.
    */
    <type.name>& operator=(const <type.name>& obj<type.name>);

    <if(type.base.Abstract)>
    virtual shared_ptr\<std::string\> ToXML() const;
    <elseif(type.Abstract)>
    virtual shared_ptr\<std::string\> ToXML() const = 0;
    <endif>
public:
    // Setter methods
    <if(!type.BinaryData)>
    <type.fields: { field |
<if(!field.Array)>
/*! Set the <field.name> parameter for class <type.name>
@param <if(!field.type.BuiltIn)>shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\><else><field.type.name><endif>
@returns void
*/
void Set<field.name;format="toUpperFirst">(<if(!field.type.BuiltIn)>shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\><else><field.type.name><endif> <field.name>);
<endif>}>
    <else>
    void SetData(byte* data, unsigned int length);
    <endif>

    // Getter methods
<type.fields: { field |
<if(!field.Array)>
    /*! Get the <field.name> parameter for class <type.name>
    @returns <if(!field.type.BuiltIn)>shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\><else><field.type.name><endif>
    */
    <if(!field.type.BuiltIn)>shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\><else><field.type.name><endif> Get<field.name;format="toUpperFirst">() const;

<else>
    /*! Get the <field.name> parameter for class <type.name>
    @returns shared_ptr\<std::vector\<<if(!field.type.BuiltIn)>shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\><else><field.type.name><endif> \> \>
    */
    shared_ptr\<std::vector\<<if(!field.type.BuiltIn)>shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\><else><field.type.name><endif> \> \> Get<field.name;format="toUpperFirst">Array() const;

<endif>}>

//methods
private:
    void Initialize();
//member variables
private:
    <type.fields: { field |
<if(!field.Array)>
    <if(!field.type.BuiltIn)>
    shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\>
    <else>
    <field.type.name><endif> <field.name>; /*!\< <field.description>  */
<else>
    shared_ptr\<std::vector\<
        <if(!field.type.BuiltIn)>
        shared_ptr\<protocol::<field.type.name;format="toUpperFirst">\>
    <else>
        <field.type.name><endif> \> \>
  <field.name>; /*!\< <field.description>  */

<endif> }>
};
typedef std::vector\<protocol::<type.name>\>  <type.name>List;
typedef shared_ptr\<protocol::<type.name>\>  <type.name>SharedPtr;
typedef shared_ptr\<std::vector\<shared_ptr\<protocol::<type.name>\> \> \> <type.name>SharedPtrList;
typedef std::vector\<shared_ptr\<protocol::<type.name>\> \> <type.name>SharedPtrArray;
}
#endif
/*! @} */
>>

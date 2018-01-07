TypeToTypeName(type) ::= <<
<if(type.simple)><type.name><else>NB_<type.name><endif>
>>
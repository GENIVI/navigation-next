ParametersCSetField(self, field, parents, prefix, suffix) ::= <<
<if(field.type.simple)><prefix><parents: {parent | <parent.name>.}><field.name> = <parents: {parent | <parent.name>.}><field.name><suffix>
<else><field.type.fields: {subField | <(self)(self, subField, [parents, field], prefix, suffix)>}><endif>
>>

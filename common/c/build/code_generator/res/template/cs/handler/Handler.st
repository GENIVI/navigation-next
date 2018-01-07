Handler(handler) ::= <<
class Tps<handler.name>Handler {

<handler.requests : { request |
	public <request.responses : { response | void Request(<request.type.name> request, Callback\<<response.type.name>\> callback);
	}> 
}>
	
}
>>

#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(std::string request) : _content(request){
	// try {
		parse();
		// tokenize();
		// setup();
	// } catch {
		// throw failedHTTPRequest
	// }
}

int	HTTPRequest::parse(){
	
}
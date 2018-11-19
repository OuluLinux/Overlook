#include "MQL.h"

namespace Ci {

CiType* CiType::Null = new CiType("null");
CiType* CiType::Void = new CiType("void");

CiArrayPtrType* CiArrayPtrType::WritableByteArray = new CiArrayPtrType(CiByteType::Value());



CiSymbol* CiBoolType::LookupMember(String name) {
	if (name == "SByte")
		return CiLibrary::SByteProperty;
		
	throw ParseException("No member " + name + " in byte");
}

CiSymbol* CiByteType::LookupMember(String name) {
	if (name == "SByte") return CiLibrary::SByteProperty;
	throw new ParseException("No member " + name + " in byte");
}

CiSymbol* CiIntType::LookupMember(String name) {
	if (name == "LowByte")
		return CiLibrary::LowByteProperty;
		
	if (name == "MulDiv")
		return CiLibrary::MulDivMethod;
		
	throw ParseException("No member " + name + " in int");
}

CiSymbol* CiStringType::LookupMember(String name) {
	if (name == "Length")
		return CiLibrary::StringLengthProperty;
		
	if (name == "Substring")
		return CiLibrary::SubstringMethod;
		
	// CharAt is available only via bracket indexing
	throw ParseException("No member " + name + " in string");
}


CiSymbol* CiArrayType::LookupMember(String name) {
	if (name == "copy_to") {
		if (this->element_type == CiByteType::Value())
			return CiLibrary::Arraycopy_toMethod;
			
		throw ParseException("copy_to available only for byte arrays");
	}
	
	if (name == "ToString") {
		if (this->element_type == CiByteType::Value())
			return CiLibrary::ArrayToStringMethod;
			
		throw ParseException("ToString available only for byte arrays");
	}
	
	throw ParseException("No member " + name + " in array");
}

CiSymbol* CiArrayStorageType::LookupMember(String name) {
	if (name == "Clear") {
	
		if (this->element_type == CiByteType::Value() || this->element_type == CiIntType::Value())
			return CiLibrary::ArrayStorageClearMethod;
			
		throw ParseException("Clear available only for byte and int arrays");
	}
	if (name == "Length")
		return new CiConst(CiIntType::Value(), new Object(this->length));
	
	//return base->LookupMember(name);
	return NULL;
}

CiProperty*	CiLibrary::LowByteProperty = new CiProperty("LowByte", CiByteType::Value());
CiProperty*	CiLibrary::SByteProperty = new CiProperty("SByte", CiIntType::Value());
CiMethod*	CiLibrary::MulDivMethod = new CiMethod(
	CiIntType::Value(), "MulDiv",
	new CiParam(CiIntType::Value(), "numerator"),
	new CiParam(CiIntType::Value(), "denominator"));
CiProperty*	CiLibrary::StringLengthProperty = new CiProperty("Length", CiIntType::Value());
CiMethod*	CiLibrary::CharAtMethod = new CiMethod(
	CiIntType::Value(), "CharAt",
	new CiParam(CiIntType::Value(), "index"));
CiMethod*	CiLibrary::SubstringMethod = new CiMethod(
	CiStringPtrType::Value(), "Substring",
	new CiParam(CiIntType::Value(), "startIndex"),
	new CiParam(CiIntType::Value(), "length"));
CiMethod*	CiLibrary::Arraycopy_toMethod = new CiMethod(
	CiType::Void, "copy_to",
	new CiParam(CiIntType::Value(), "sourceIndex"),
	new CiParam(CiArrayPtrType::WritableByteArray, "destinationArray"),
	new CiParam(CiIntType::Value(), "destinationIndex"),
	new CiParam(CiIntType::Value(), "length"));
CiMethod*	CiLibrary::ArrayToStringMethod = new CiMethod(
	CiStringPtrType::Value(), "ToString",
	new CiParam(CiIntType::Value(), "startIndex"),
	new CiParam(CiIntType::Value(), "length"));
CiMethod*	CiLibrary::ArrayStorageClearMethod = new CiMethod(
	CiType::Void, "Clear", true); // IsMutator = true
            
            
CiDelegate* CiMethodCall::Signature() {
	return method != NULL ? this->method->signature : (CiDelegate*) this->obj->Type();
}


}

#pragma once
#ifndef ECSMESSAGE
#define ECSMESSAGE
#ifdef	ENGINE_EXE_EXPORT
#define DT_ENGINE_API 
#elif	ENGINE_DLL_EXPORT 
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include <stdio.h>
#include <string>
#include <utility>
#include <memory>

using namespace std;


/** Payload concept/interface. */
struct Command_Base {
	virtual ~Command_Base() {};
	virtual const char * GetTypeID() const = 0;
};
/** The actual payload container. */
template <typename DATA_TYPE>
struct Command_Payload : Command_Base {
	~Command_Payload() {}
	Command_Payload(const DATA_TYPE & t) : m_data(t) {};
	const char * GetTypeID() const { return typeid(DATA_TYPE).name(); }
	DATA_TYPE m_data;
};
struct ECS_Command {
	template <typename T> ECS_Command(const T & obj) : m_data(std::make_unique<Command_Payload<T>>(obj)) {	}
	/** Checks to see if the payload is of the type supplied.
	* @param	<T>			any data type to check
	* @return				true if the payload is of the type requested, false otherwise */
	template <typename T> bool isType() const { return strcmp(m_data->GetTypeID(), typeid(T).name()) == 0; }
	/* Cast this payload to the type specified.
	* @param	<T>			any data type to typecast the payload to
	* @return				type-casted payload */
	template <typename T> const T & toType() const { return (dynamic_cast<Command_Payload<T>*>(m_data.get()))->m_data; };
	std::unique_ptr<Command_Base> m_data;
};

#endif // ECSMESSAGE

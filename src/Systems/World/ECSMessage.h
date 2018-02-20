#pragma once
#ifndef ECSMESSAGE
#define ECSMESSAGE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\World\ECSdefines.h"
#include <stdio.h>
#include <string>
#include <utility>
#include <memory>

using namespace std;
struct PayloadModel;

/**
 * An object for sending data between components and/or entities.
 * Is a container for any data type.
 * Data type can be manually checked, and specifically type-casted on request.
 **/
class DT_ENGINE_API ECSmessage
{
public:
	// (de)Constructors
	/** Destroy the message. */
	~ECSmessage() { }
	/** Construct a message. 
	 * @param	commandID	an unsigned integer representing a command in the target the message will be received by 
	 * @param	obj			any object of type <DATA_TYPE> that will constitute the payload to be sent as the message
	 * @param	senderID	an optional parameter indicating who the sender is, defaults null with -1 as the ID
	 * @param	<DATA_TYPE>	any data type to send as a message payload */
	template <typename DATA_TYPE>
	ECSmessage(const unsigned int & commandID, const DATA_TYPE & obj, const ECShandle & senderID = ECShandle("", -1)) :
		m_commandID(commandID),
		m_payload(std::make_unique<PayloadModel<DATA_TYPE>>(obj)),
		m_senderID(senderID)
	{}

	
	// Public Methods
	/** Returns the sender ID of this message.
	 * @return				ECShandle of this message */
	ECShandle GetSenderID() const { return m_senderID; };
	/** Returns the command ID of this message.
	 * @return				unsigned int command ID of this message */
	unsigned int GetCommandID() const { return m_commandID; }
	/** Returns the payload in the format requested.
	 * @brief				typecasts the payload and returns it. Doesn't check if it is safe to do so (need to call isOfType first)
	 * @param	<T>			any data type to typecast the payload to
	 * @return				type-casted payload */
	template <typename T> const T & GetPayload() const {	return (dynamic_cast<PayloadModel<T>*>(m_payload.get()))->GetData(); };	
	/** Checks to see if the payload is of the type supplied.
	 * @param	<T>			any data type to check
	 * @return				true if the payload is of the type requested, false otherwise */
	template <typename T> bool IsOfType() const { return strcmp(m_payload->GetTypeID(), typeid(T).name()) == 0; }


private:
	// Private Nested Classes
	/** Payload concept/interface */
	struct PayloadConcept {
		virtual ~PayloadConcept() {};
		virtual const char * GetTypeID() = 0;
	};
	/** The actual payload container.
	 * This class is abstract and holds the data for every message. */
	template <typename DATA_TYPE>
	struct PayloadModel : PayloadConcept {
		// (de)Constructors
		~PayloadModel() {}
		PayloadModel(const DATA_TYPE & t) { m_data = t; }

		// Public Methods
		const char * GetTypeID() { return typeid(DATA_TYPE).name(); }
		const DATA_TYPE &GetData() const { return m_data; }


	private:
		// Private Members
		DATA_TYPE m_data;
	};


	// Private Attributes
	unsigned int m_commandID;
	ECShandle m_senderID;
	std::unique_ptr<PayloadConcept> m_payload;
};

#endif // ECSMESSAGE

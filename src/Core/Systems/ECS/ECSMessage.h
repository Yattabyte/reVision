/*
	ECSmessage

	- A class for sending data to and from components and entities.
	- Is a container for any data type
	- Data type can be manually checked, and specifically typecasted on request
*/

#pragma once
#ifndef ECSMESSAGE
#define ECSMESSAGE
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Systems\ECS\ECSdefines.h"
#include <stdio.h>
#include <string>
#include <utility>
#include <memory>

using namespace std;

class DT_ENGINE_API ECSmessage
{
private:
	// The payload interface
	struct PayloadConcept {
		virtual ~PayloadConcept() {};
		virtual const char* GetTypeID() = 0;
	};

	// The specific class that will hold our payload's data
	template <typename DATA_TYPE>
	struct PayloadModel : PayloadConcept {
		PayloadModel(const DATA_TYPE& t) { m_data = t; }
		~PayloadModel() {}
		const char* GetTypeID() {
			return typeid(DATA_TYPE).name();
		}
		const DATA_TYPE &GetData() const { return m_data; }
	private:
		DATA_TYPE m_data;
	};

	unsigned int m_commandID;
	ECShandle m_senderID;
	std::unique_ptr<PayloadConcept> m_payload;

public:
	// Constructor, generates a container to hold our object
	template <typename DATA_TYPE>
	ECSmessage(const unsigned int &commandID, const DATA_TYPE& obj, const ECShandle &senderID = ECShandle("", -1)) :
		m_commandID(commandID),
		m_payload(std::make_unique<PayloadModel<DATA_TYPE>>(obj)),
		m_senderID(senderID)
	{}
	~ECSmessage() { }

	// Returns the sender ID 
	ECShandle GetSenderID() const { return m_senderID; };
	// Returns the command ID
	unsigned int GetCommandID() const { return m_commandID; }
	// Returns the payload in the format requested
	template <typename T> const T &GetPayload() const {	return (dynamic_cast<PayloadModel<T>*>(m_payload.get()))->GetData(); };	
	// Checks to see if the payload is of the type supplied
	template <typename T> bool IsOfType() const { return strcmp(m_payload->GetTypeID(), typeid(T).name()) == 0; }
};

#endif // ECSMESSAGE

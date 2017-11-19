/*
	ECSMessage

	- A class for sending data to and from components and entities.
	- Is useless in its basic form. Must be expanded upon and given a unique ID.
	- Recievers will check the ID and typecast this to the intended form.
*/

#pragma once
#ifndef ECSMESSAGE
#define ECSMESSAGE
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

typedef std::pair<char*, unsigned int> ECSHandle;

class ECSMessage
{
public: 
	DELTA_CORE_API ~ECSMessage() {};
	DELTA_CORE_API ECSMessage() : m_typeID(-1), m_senderID(ECSHandle("", -1)), m_targetID(ECSHandle("", -1)) {};
	DELTA_CORE_API unsigned int GetTypeID() const { return m_typeID; };
	DELTA_CORE_API ECSHandle GetSenderID() const { return m_senderID; };
	DELTA_CORE_API void SetSenderID(const ECSHandle &sender) { m_senderID = sender; };
	DELTA_CORE_API ECSHandle GetTargetID() const { return m_targetID; };
	DELTA_CORE_API void SetTargetID(const ECSHandle &target) { m_targetID = target; };

protected:
	unsigned int m_typeID;
	ECSHandle m_senderID, m_targetID;
};

#endif // ECSMESSAGE

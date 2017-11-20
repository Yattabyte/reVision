/*
	ECSmessage

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

class DELTA_CORE_API ECSmessage
{
public: 
	~ECSmessage() {};
	ECSmessage() : m_senderID(ECSHandle("", -1)), m_targetID(ECSHandle("", -1)) {};
	virtual const char* GetTypeID() { return 0; };
	ECSHandle GetSenderID() const { return m_senderID; };
	void SetSenderID(const ECSHandle &sender) { m_senderID = sender; };
	ECSHandle GetTargetID() const { return m_targetID; };
	void SetTargetID(const ECSHandle &target) { m_targetID = target; };

protected:
	ECSHandle m_senderID, m_targetID;
};

#endif // ECSMESSAGE

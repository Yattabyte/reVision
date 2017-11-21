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

#include <memory>

typedef std::pair<char*, unsigned int> ECSHandle;


class DELTA_CORE_API ECSmessage
{
private:
	struct PayloadConcept {
		virtual ~PayloadConcept() {}
		virtual const char* GetTypeID() { return 0; }
	};
	template <typename T>
	struct PayloadModel : PayloadConcept {
		PayloadModel(const T& t) { m_data = t; }
		virtual ~PayloadModel() {}
		virtual const char* GetTypeID() {
			return typeid(T).name();
		}
		const T &GetData() const { return m_data; }
	private:
		T m_data;
	};

	ECSHandle m_senderID, m_targetID;
	PayloadConcept *m_payload;


public:
	template< typename T > 
	ECSmessage(const T& obj) : m_payload(new PayloadModel<T>(obj)) {}
	~ECSmessage() { delete m_payload; }

	ECSHandle GetSenderID() const { return m_senderID; };
	void SetSenderID(const ECSHandle &sender) { m_senderID = sender; };
	ECSHandle GetTargetID() const { return m_targetID; };
	void SetTargetID(const ECSHandle &target) { m_targetID = target; };

	template< typename T >
	const T &GetPayload() const { 
		auto cast = dynamic_cast<PayloadModel<T>*>(m_payload)->GetData();
		return cast;
	};

	template< typename T>
	bool IsOfType() const { return strcmp(m_payload->GetTypeID(), typeid(T).name()) == 0; }
};

#endif // ECSMESSAGE

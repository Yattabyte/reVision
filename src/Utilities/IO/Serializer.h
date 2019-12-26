#pragma once
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <algorithm>
#include <optional>
#include <map>
#include <string>
#include <vector>


/** A utility class for serializing labeled values. */
class Serializer {
public:
	/** Serialize a set of labeled value pairs to a char buffer.
	@note Input values must be a std::pair<std::string, X> where X is the value to be serialized!
	@param	<FirstMember, ...RemainingMembers>	variadic list of any value to serialized (auto-deducible).
	@param	first			the first value to serialize.
	@param	...rest			the rest of the values to serialize.
	@return					a char buffer containing serialized data. */
	template <typename FirstMember, typename ...RemainingMembers>
	inline static std::vector<char> Serialize_Set(const FirstMember& first, const RemainingMembers& ...rest) {
		// Retrieve the first member's serialized data
		const auto& [MemberName, MemberContainer] = first;
		auto memberData = Serialize_Value(MemberName, MemberContainer);

		// For each remaining member of the parameter pack, recursively call this function, appending the results
		if constexpr (sizeof...(rest) > 0) {
			const auto nextMemberData = Serialize_Set(rest...);
			memberData.insert(memberData.end(), nextMemberData.begin(), nextMemberData.end());
		}

		return memberData;
	}
	/** De-serialize a char buffer into a set of labeled value pairs.
	@note Input values must be a std::pair<std::string, *X> where X is a pointer to the value to be assigned!
	@param	<...Members>	variadic list of all value pairs to updated (auto-deducible).
	@param	memberData		a char buffer containing serialized data.
	@param	...members		the list of value pairs to update. */
	template <typename ...Members>
	inline static void Deserialize_Set(const std::vector<char>& memberData, const Members& ...members) {
		// Ensure the data buffer is valid
		if (memberData.size()) {
			std::map<std::string, std::vector<char>> memberMap;

			// Populate map with member info from buffer data
			constexpr static auto Fill_Member_Map = [](auto& memberMap, const auto& memberData) {
				size_t index(0ULL), prevIndex(0ULL);
				while (index < memberData.size()) {
					// Get the memory structure
					struct Memory_Structure {
						int struct_size;
						char payload_name[MAX_NAME_CHARS]{ '\0' };
						int payload_size;
					};
					const auto& memStruct = *reinterpret_cast<const Memory_Structure*>(&memberData[index]);
					const std::string memory_payload_string(memStruct.payload_name, MAX_NAME_CHARS);
					if (memStruct.struct_size == 0 || memory_payload_string.empty())
						break;

					// Skip getting the memory payload's data
					// char* memory_payload_data = const_cast<char*>(&memberData[index + sizeof(Memory_Structure)]);
					index += memStruct.struct_size;

					// Insert clamped string with memory data into map
					memberMap.insert({ memory_payload_string.c_str(), std::vector<char>(memberData.begin() + prevIndex, memberData.begin() + index) });
					prevIndex = index;
				}
			};
			Fill_Member_Map(memberMap, memberData);

			// Iterate through the expected member names
			Search_And_Assign_Members(memberMap, members...);
		}
	}
	/** Serialize a labeled pair of data into a char buffer.
	@param	<T>				the data type to serialize (auto-deducible).
	@param	name			the label for the data (i.e. the variable name).
	@param	data			the value to serialize.
	@return					a char buffer containing serialized data. */
	template <class T>
	inline static std::vector<char> Serialize_Value(const std::string& name, const T& data) {
		// For convenience sake, wrap our output data into a memory-copyable struct
		struct Memory_Structure {
			int struct_size = (int)(sizeof(Memory_Structure));
			char payload_name[MAX_NAME_CHARS]{ '\0' };
			int payload_size = (int)(sizeof(T));
			char payload_data[sizeof(T)]{0};

			/** Fill this memory structure. */
			Memory_Structure(const std::string& name, const T& data) {
				// Copy-in the variable name, clamped to a max of MAX_NAME_CHARS)
				std::copy(name.begin(), name.size() <= (size_t)(MAX_NAME_CHARS) ? name.end() : name.begin() + MAX_NAME_CHARS, std::begin(payload_name));
				// Copy-in the variable data
				*reinterpret_cast<T*>(&payload_data[0]) = data;
			}
		} const outputData(name, data);

		// Allocate a buffer for our data, copy into it, and write pass it back
		std::vector<char> dataBuffer(sizeof(Memory_Structure));
		*reinterpret_cast<Memory_Structure*>(&dataBuffer[0]) = outputData;
		return dataBuffer;
	}
	/** De-serialize a char buffer into a labeled pair of data.
	@param	<T>				the data type to de-serialize.
	@param	dataBuffer		a char buffer containing serialized data.
	@return					an optional pair containing a label and value <T> if successful. */
	template <class T>
	static std::optional<std::pair<std::string, T>> Deserialize_Value(const std::vector<char>& dataBuffer) {
		// The expected structure of the input data
		struct Memory_Structure {
			const int struct_size = (int)sizeof(Memory_Structure);
			char payload_name[MAX_NAME_CHARS]{ '\0' };
			const int payload_size = (int)sizeof(T);
			char payload_data[sizeof(T)];
		};

		// Ensure the data buffer is of the expected size
		if (dataBuffer.size() == (int)sizeof(Memory_Structure)) {
			// Cast the memory back into the structure
			const auto& inputData = *reinterpret_cast<const Memory_Structure*>(&dataBuffer[0]);
			// Ensure internal memory size matches
			if (dataBuffer.size() == inputData.struct_size) {
				const std::string name(inputData.payload_name, MAX_NAME_CHARS);
				const T& data = *reinterpret_cast<const T*>(&inputData.payload_data[0]);
				return { { name,data } };
			}
		}
		return {};
	}


private:
	// Private Methods
	/** Given a filled member-map, search for the input label and assign the mapped value.
	@param	<FirstMember, ...RemainingMembers>	variadic list of any value to de-serialized (auto-deducible).
	@param	memberMap		a map of labels to serialized data.
	@param	first			the first value to de-serialize.
	@param	...rest			the rest of the values to de-serialize. */
	template <typename FirstMember, typename ...RemainingMembers>
	inline static void Search_And_Assign_Members(const std::map<std::string, std::vector<char>>& memberMap, FirstMember& first, RemainingMembers& ...rest) {
		// Try to find the first member of this parameter pack in the member map
		auto& [MemberName, MemberPointer] = first;
		const auto memberNameSpot = memberMap.find(std::string(MemberName));
		if (memberNameSpot != memberMap.end()) {
			// Found member, de-serialize data
			if (const auto qwe = Deserialize_Value<typename std::remove_pointer<decltype(MemberPointer)>::type>(memberMap.at(MemberName)))
				*MemberPointer = qwe->second; // assign it
		}

		// Repeat for remaining members
		if constexpr (sizeof...(rest) > 0)
			Search_And_Assign_Members(memberMap, rest...);
	}


	// Private Variables
	constexpr static int MAX_NAME_CHARS = 32;
};

template <>
inline std::vector<char> Serializer::Serialize_Value(const std::string& name, const std::string& data) {
	// For convenience sake, wrap our output data into a memory-copyable struct
	struct Memory_Structure {
		int struct_size = (int)sizeof(Memory_Structure);
		char payload_name[MAX_NAME_CHARS]{ '\0' };
		int payload_size;

		/** Fill this memory structure. */
		Memory_Structure(const std::string& name, const std::string& data) {
			// Update size variables with data size
			payload_size = (int)(sizeof(char) * data.size());
			struct_size += payload_size;

			// Copy-in the variable name, clamped to a max of MAX_NAME_CHARS)
			std::copy(name.begin(), name.size() <= (size_t)(MAX_NAME_CHARS) ? name.end() : name.begin() + MAX_NAME_CHARS, std::begin(payload_name));
		}
	} const outputData(name, data);

	// Allocate a buffer for our data, copy into it, and write pass it back
	std::vector<char> dataBuffer(outputData.struct_size);
	*reinterpret_cast<Memory_Structure*>(&dataBuffer[0]) = outputData;
	// Copy-in the variable data
	std::copy(data.begin(), data.end(), dataBuffer.begin() + sizeof(Memory_Structure));
	return dataBuffer;
}
template <>
inline std::optional<std::pair<std::string, std::string>> Serializer::Deserialize_Value(const std::vector<char>& dataBuffer) {
	// The expected structure of the input data
	struct Memory_Structure {
		int struct_size = (int)sizeof(Memory_Structure);
		char payload_name[MAX_NAME_CHARS]{ '\0' };
		int payload_size;
	};

	// Ensure the data buffer is of the expected size
	if (dataBuffer.size() >= (int)sizeof(Memory_Structure)) {
		// Cast the memory back into the structure
		const auto& inputData = *reinterpret_cast<const Memory_Structure*>(&dataBuffer[0]);
		// Ensure internal memory size matches
		if (dataBuffer.size() == inputData.struct_size) {
			const std::string name(inputData.payload_name, MAX_NAME_CHARS);
			std::string payload_string(&dataBuffer[sizeof(Memory_Structure)], inputData.payload_size);
			return { { name, payload_string } };
		}
	}
	return {};
}

#endif // SERIALIZER_H
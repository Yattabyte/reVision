#pragma once
#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H


class Engine;

/** An interface for engine modules to implement. */
class Engine_Module {
public:
	// (de)Constructors
	~Engine_Module() = default;
	Engine_Module() = default;


	/** Initialize the module. */
	inline virtual void initialize(Engine * engine) {
		m_engine = engine;
	};
	inline virtual void frameTick(const float & deltaTime) {
	};


protected:
	Engine * m_engine = nullptr;
};

#endif // ENGINE_MODULE_H
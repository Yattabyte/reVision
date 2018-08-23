#pragma once
#ifndef ENGINE_MODULE_H
#define ENGINE_MODULE_H


class Engine;

/** An interface for engine modules to implement. */
class Engine_Module {
public:
	// (de)Constructors
	~Engine_Module() {}
	Engine_Module(Engine * engine) : m_engine(engine) {}


	/** Initialize the module. */
	virtual void initialize() {};


protected:
	Engine * m_engine = nullptr;
};

#endif // ENGINE_MODULE_H
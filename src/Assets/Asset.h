#pragma once
#ifndef	ASSET
#define	ASSET
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include <shared_mutex>
#include <vector>

using namespace std;
class Asset_Observer;
class Asset;
typedef shared_ptr<Asset> Shared_Asset;


/**
 * An abstract base-class for assets.
 * @brief	Represents some form of data to be loaded from disk, such as shaders, models, levels, and sounds.
 * @note	is an abstract class instead of interface to reduce redundant code.
 * @usage	Should be created once, and its pointer passed around using shared pointers.
 **/
class DT_ENGINE_API Asset
{
public:
	// (de)Constructors
	/** Destroy the asset only when all references are destroyed. */
	~Asset();
	/** Create asset that uses the specified file-path. */
	Asset(const string & filename = "");


	// Public Methods	
	/** Gets the file name of this asset.
	 * @return				the file name belonging to this asset */
	string getFileName() const;
	/** Sets the file name of this asset.
	 * @param	filename	the file name to set this asset to */
	void setFileName(const string & filename);
	/** Adds a state observer/listener to this asset.
	 * @brief				adds asset observers that want to be know when this asset finishes finalizing.
	 * @param	observer	the observer to add to this asset */
	void addObserver(Asset_Observer * observer);
	/** Removes a state observer/listener from this asset.
	 * @brief				removes the observer specified from this asset.
	 * @note				will remove all instances from this asset, or none if it doesn't exist. 
	 * @param	observer	the observer to remove from this asset*/
	void removeObserver(Asset_Observer * observer);
	/** Returns a UNIQUE asset type identifier. Each sub-class should have their own
  	 * @todo	Delete this and change the system to use const char * keys */
	static int Get_Asset_Type();
	/** Returns whether or not this asset has completed finalizing.
	 * @note				Virtual, each asset can re-implement if they have specific finalizing criteria.
	 * @return				true if this asset has finished finalizing, false otherwise. */
	virtual bool existsYet();
	/** Performs final data processing.
	 * @note	Virtual, each asset can re-implement if they have specific finalizing criteria. */
	virtual void finalize();


	// Public Attributes
	shared_mutex m_mutex;	/** public mutex, to encourage safe access of asset. */


protected:
	// Protected Attributes
	bool m_finalized;
	string m_filename;
	vector<Asset_Observer*> m_observers;
};

/**
 * An abstract class used to tailor a specific response to an asset completing both initializing and finalizing.\n
 * To be appended to an asset's observer list, and will be iterated through at its discretion.
 * @todo	Maybe make this a template or something, try to make the destructor code not redundant.
 **/
class DT_ENGINE_API Asset_Observer
{
public:
	/** Constructor. Takes the asset, and calls its addObserver method on *this*. */
	Asset_Observer(Asset * asset) { asset->addObserver(this); }
	/** Virtual destructor. To be used in removing observer from the asset. */
	virtual ~Asset_Observer() {}; 
	/** To be called when the asset finishes finalizing. */
	virtual void Notify_Finalized() = 0;
};

/**
 * An abstract class for assets work orders.
 * @brief	Each asset should implement a specific work order specialized for their own data.
 **/
class DT_ENGINE_API Work_Order
{
public:
	// (de)Constructor
	/** Generic default constructor .*/
	Work_Order() {};
	/** Virtual destructor. */
	virtual ~Work_Order() {};
	

	// Public Methods
	/** Begins reading and parsing an asset's data from disk. 
	 * @note	This is designed to be multi-threaded if the asset can support it. */
	virtual void initializeOrder() = 0;
	/** Finishes remaining operations needed to finalize an asset.
	 * @note	Since many assets require GPU synchronization, this step is done in the main thread. */
	virtual void finalizeOrder() = 0;
};

#endif // ASSET
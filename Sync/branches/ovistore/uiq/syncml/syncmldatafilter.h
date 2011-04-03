/** 
@file

Copyright (c) 2002-2005 Symbian Software Ltd.	All rights reserved.
*/

#ifndef SYNCMLDATAFILTER_H
#define SYNCMLDATAFILTER_H

#include <s32strm.h>

class CSyncMLFilterDef;
class CSyncMLFilter;

/**
Indicates how to combine multiple filters within a task.

@publishedAll
*/
enum TSyncMLFilterMatchType
	{
	ESyncMLMatchDefault,	// use default of Data Provider
	ESyncMLMatchAnd,		// AND all the filters
	ESyncMLMatchOr,			// OR all the filters
	};

/**
Action for the server to take when applying the filters, if the 
new filters should be the only ones used and any previous synchronised data 
diregarder, or otherwise.

@publishedAll
*/
enum TSyncMLFilterOutsideInclusionType
	{
	ESyncMLOutsideDefault,		// use default of Data Provider
	ESyncMLOutsideInclusive,	// make filters EXCLUSIVE
	ESyncMLOutsideExclusive,	// make filters INCLUSIVE
	};

/**
The data type of the filter property.

@publishedAll
*/
enum TSyncMLFilterPropertyDataType
	{
	ESyncMLDataTypeNull,
	ESyncMLDataTypeBool,
	ESyncMLDataTypeNumber,
	ESyncMLDataTypeDateTime,
	ESyncMLDataTypeText8,
	ESyncMLDataTypeText16,
	};

/**
Specifies if the filter indicates filtering of records or of particular fields.

@publishedAll
*/
enum TSyncMLFilterLevelType
	{
	ESyncMLFilterRecordLevel,
	ESyncMLFilterFieldLevel,
	};

/**
Class that defines a particular property of a filter. A filter may have between none and many properties.

@publishedAll
*/
class CSyncMLFilterProperty : public CBase
	{
public:
	IMPORT_C static CSyncMLFilterProperty* NewL(RReadStream& aStream);
	IMPORT_C static CSyncMLFilterProperty* NewL(const TDesC& aName, TSyncMLFilterPropertyDataType aDataType);
	IMPORT_C static CSyncMLFilterProperty* NewL(const CSyncMLFilterProperty& aProperty);
	
	IMPORT_C void ExternalizeL(RWriteStream& aStream) const;
	IMPORT_C virtual ~CSyncMLFilterProperty();

	IMPORT_C const TDesC& Name() const;
	IMPORT_C TSyncMLFilterPropertyDataType DataType() const;
	IMPORT_C TUint MaxTextLength() const;

	IMPORT_C TInt PresetValueCount() const;
	IMPORT_C const TDesC16& PresetValueText16L(TInt aIndex) const;
	IMPORT_C const TDesC8& PresetValueText8L(TInt aIndex) const;
	IMPORT_C TInt PresetValueIntL(TInt aIndex) const;
	IMPORT_C TBool PresetValueBoolL(TInt aIndex) const;
	IMPORT_C TTime PresetValueDateTimeL(TInt aIndex) const;

	IMPORT_C TBool MustUsePresetValue() const;
	IMPORT_C const RArray<TInt>& DefaultPresetIndices() const;

	IMPORT_C TBool SupportsMultipleSelection() const;
	
	IMPORT_C void SetMaxTextLengthL(TUint);
	IMPORT_C void ClearPresetValuesL();
	IMPORT_C void AddPresetValueText16L(const TDesC16& aValue);
	IMPORT_C void AddPresetValueText8L(const TDesC8& aValue);
	IMPORT_C void AddPresetValueIntL(TInt aValue);
	IMPORT_C void AddPresetValueBoolL(TBool aValue);
	IMPORT_C void AddPresetValueTTimeL(TTime aValue);
	IMPORT_C void SetMustUsePresetValue(TBool aValue);
	IMPORT_C void ClearDefaultPresetIndicesL();
	IMPORT_C void AddDefaultPresetIndexL(TInt aIndex);
	IMPORT_C void SetSupportMultipleSelectionL(TBool aValue);
	
	IMPORT_C const TDesC16& PropertyValueText16L() const;
	IMPORT_C const TDesC8& PropertyValueText8L() const;
	IMPORT_C TInt PropertyValueIntL() const;
	IMPORT_C TBool PropertyValueBoolL() const;
	IMPORT_C TTime PropertyValueDateTimeL() const;

	IMPORT_C void SetPropertyValueText16L(const TDesC16& aValue);
	IMPORT_C void SetPropertyValueText8L(const TDesC8& aValue);
	IMPORT_C void SetPropertyValueIntL(TInt aValue);
	IMPORT_C void SetPropertyValueBoolL(TBool aValue);
	IMPORT_C void SetPropertyValueDateTimeL(TTime aValue);

	IMPORT_C void ClearPropertyValueL();

	IMPORT_C TInt SelectedValueIndicesCount() const;
	IMPORT_C TInt SelectedValueIndex(TInt aIndex) const;

	IMPORT_C void ClearSelectedValueIndices();
	IMPORT_C void ClearSelectedValueIndexL(TInt aIndex);
	IMPORT_C void AddSelectedValueIndexL(TInt aIndex);

	IMPORT_C TBool PresetValueSet() const;
	IMPORT_C TBool PropertyValueSet() const;

private:
	CSyncMLFilterProperty();
	void ConstructL(RReadStream& aStream);
	void ConstructL(const TDesC& aName, TSyncMLFilterPropertyDataType aDataType);
	void ConstructL(const CSyncMLFilterProperty& aProperty);


	__DECLARE_TEST;	
private:
	HBufC* iName;									// User readable name for the property
	TSyncMLFilterPropertyDataType iDataType;		// The data type of the property
	TUint iMaxTextLength;							// The maximum length of value/preset strings for a Text8/Text16 object
	
	// Preset value members
	RPointerArray<HBufC16> iPresetValueText16List;	// The array holding the presets to display for this property
	RPointerArray<HBufC8> iPresetValueText8List;	// The array holding the presets to display for this property
	RArray<TInt> iPresetValueIntBoolList;			// The array holding the presets to display for this property
	RArray<TTime> iPresetValueDateTimeList;			// The array holding the presets to display for this property
	TBool iMustUsePresetValue; 						// ETrue if the property must be set as a pre-set.
	RArray<TInt> iDefaultPresetIndices; 			// Set of default pre-set values - if empty then no default
	TBool iSupportsMultipleSelection; 				// ETrue if the property can have multiple pre-set values selected

	// Single value members	
	TBool iValueSet; 								// ETrue when value set, EFalse until then.
	// Actual value entered or selected by the user.
	HBufC16 *iValueText16;							// Single value user setting
	HBufC8 *iValueText8;							// Single value user setting
	TInt iValueIntBool;								// Single value user setting
	TTime iValueDateTime;							// Single value user setting

	// If multi-values selected then don't use values above.
	RArray<TInt> iSelectedValueIndices; 			// Array of user selected indexes into preset array

	TBool iScratch;									// Distinguish how the object was created and therefore what can be set on it
	};


/** The unique Identifier of a filter. The ID is unique only within the task that owns the filter.
@publishedAll
*/
typedef TInt TSmlFilterId;

/** Class representing a filter. It is created by the Data Provider and then used and modified by others 
as a copy of the original one - can be a copy comming from the filter directly or from a stream.

@publishedAll
*/
class CSyncMLFilter : public CBase
	{
public:
	IMPORT_C static CSyncMLFilter* NewL(const CSyncMLFilter& aFilter);
	IMPORT_C static CSyncMLFilter* NewL(RReadStream& aStream);
	IMPORT_C static CSyncMLFilter* NewL(TSmlFilterId aId, TSyncMLFilterLevelType aFilterLevel,
	                		const TDesC& aDisplayName, const TDesC& aDisplayDescription );

	IMPORT_C virtual ~CSyncMLFilter();
	IMPORT_C void ExternalizeL(RWriteStream& aStream) const;

	IMPORT_C TSmlFilterId FilterId() const;
	IMPORT_C const TDesC& DisplayName() const;
	IMPORT_C const TDesC& DisplayDescription() const;
	IMPORT_C const RPointerArray<CSyncMLFilterProperty>& Properties() const;
	IMPORT_C void SetEnabledL(TBool aEnabled);
	IMPORT_C TBool Enabled() const;
	IMPORT_C TSyncMLFilterLevelType FilterLevel() const;
  
	IMPORT_C void ClearPropertiesL();
	IMPORT_C void AddPropertyL(CSyncMLFilterProperty* aProperty);

	IMPORT_C TInt PropertyCount() const;
	IMPORT_C CSyncMLFilterProperty& PropertyL(TInt aIndex) const;

	IMPORT_C void UnsetAllPropertyValuesL();
	
private:
	CSyncMLFilter();
	void ConstructL(const CSyncMLFilter& aFilter);
	void ConstructL(RReadStream& aStream);
	void ConstructL(TSmlFilterId aId, TSyncMLFilterLevelType aFilterLevel,
	                const TDesC& aDisplayName, const TDesC& aDisplayDescription );

	__DECLARE_TEST;
private:
	TSmlFilterId iFilterId;							// An Id for this filter, unique to the data provider
	TSyncMLFilterLevelType iFilterLevel;			// Is this filter a Field or Record filter?
	HBufC* iDisplayName;							// User readable display name
	HBufC* iDisplayDescription;						// User readable display description
	RPointerArray<CSyncMLFilterProperty> iProperties;	// This class owns 0..n properties
	TBool iEnabled;									// Has this filter been enabled?
	};

#endif // SYNCMLDATAFILTER_H

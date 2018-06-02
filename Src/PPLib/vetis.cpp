// VETIS.CPP
// Copyright (c) A.Sobolev 2017, 2018
// @codepage UTF-8
// Модуль для взаимодействия с системой Меркурий (интерфейс ВЕТИС)
//
//
#include <pp.h>
#pragma hdrstop
#include <ppsoapclient.h>

struct VetisBusinessEntity;

struct VetisErrorEntry {
	VetisErrorEntry & Z()
	{
		Item.Z();
		Code.Z();
		Qualifier.Z();
		return *this;
	}
	SString Item;
	SString Code;
	SString Qualifier;
};

struct VetisFault { 
	enum {
		tUndef = 0,
		tAccessDenied = 1,
		tEntityNotFound,
		tIncorrectRequest,
		tInternalService,
		tUnknownServiceId,
		tUnsupportedApplicationDataType
	};
	int    Type;
	SString Message;
	TSCollection <VetisErrorEntry> ErrList;
};

struct VetisEntityList {
	VetisEntityList() : Count(0), Total(0), Offset(0), Flags(0)
	{
	}
	enum {
		fHasMore = 0x0001
	};
	int    Count;
	int64  Total;
	int    Offset;
	long   Flags;
};

struct VetisGenericEntity {
	VetisGenericEntity()
	{
	}
	S_GUID Uuid;
};

struct VetisEnterpriseActivity : public VetisGenericEntity {
	SString Name;
};

struct VetisGenericVersioningEntity : public VetisGenericEntity {
	VetisGenericVersioningEntity() : VetisGenericEntity(), Flags(0), Status(0), CreateDate(ZERODATETIME), UpdateDate(ZERODATETIME)
	{
	}
	S_GUID Guid;
	enum {
		fActive = 0x0001,
		fLast   = 0x0002
	};
	long   Flags;
	int    Status; // VersionStatus
	LDATETIME CreateDate;
	LDATETIME UpdateDate;
	S_GUID Previous;
	S_GUID Next;
};

struct VetisNamedGenericVersioningEntity : public VetisGenericVersioningEntity {
	SString Name;
};

struct VetisCountry {
	SString Name;
	SString FullName;
	SString EnglishName;
	SString Code;
	SString Code3;
};

struct VetisFederalDistrict {
	SString FullName;
	SString ShortName;
	SString Abbreviation;
};

struct VetisAddressObjectView {
	VetisAddressObjectView() : Flags(0)
	{
	}
	SString Name;
	SString EnglishName;
	SString View;
	SString RegionCode;
	SString Type;
	S_GUID CountryGUID;
	enum {
		fHasStreets = 0x0001
	};
	long   Flags;
};

struct VetisDistrict : public VetisAddressObjectView {
	VetisDistrict() : VetisAddressObjectView()
	{
	}
	S_GUID RegionGUID;
};

struct VetisLocality : public VetisAddressObjectView {
	VetisLocality::VetisLocality() : VetisAddressObjectView()
	{
	}
	S_GUID RegionGUID;
	S_GUID DistrictGUID;
	S_GUID CityGUID;
};

struct VetisStreet : public VetisAddressObjectView {
	VetisStreet() : VetisAddressObjectView()
	{
	}
	S_GUID LocalityGUID;
};

struct VetisAddress {
	VetisCountry Country;
	VetisFederalDistrict FederalDistrict;
	VetisAddressObjectView Region;
	VetisDistrict District;
	VetisLocality Locality;
	VetisLocality SubLocality;
	VetisStreet Street;
	SString House;
	SString Building;
	SString Room;
	SString PostIndex;
	SString PostBox;
	SString AdditionalInfo;
	SString AddressView;
	SString EnAddressView;
};

struct VetisOrganization {
	SString ID;
	SString Name;
	VetisAddress Address;
};

enum VetisDocType {
	vetisdoctypTRANSPORT = 0,
	vetisdoctypPRODUCTIVE = 1, 
	vetisdoctypRETURNABLE = 2, 
	vetisdoctypINCOMING = 3,
	vetisdoctypOUTGOING = 4
};

enum VetisDocStatus {
	vetisdocstCREATED = 0, 
	vetisdocstCONFIRMED = 1,
	vetisdocstWITHDRAWN = 2, 
	vetisdocstUTILIZED = 3, 
	vetisdocstFINALIZED = 4
};

struct VetisDocument : public VetisGenericEntity {
	VetisDocument() : VetisGenericEntity(), IssueDate(ZERODATE), DocumentType(0)
	{
	}
	SString Name;
	SString Form;
	SString IssueSeries;
	SString IssueNumber;
	LDATE  IssueDate;
	int    DocumentType;
	VetisOrganization Issuer;
	SString For_; 
	SString Qualifier; // ?
};

struct VetisUserAuthority {
	VetisUserAuthority() : Granted(0)
	{
	}
	SString ID;
	SString Name;
	int    Granted; // bool
};

struct VetisUser {
	VetisUser();
	~VetisUser();
	VetisUser & FASTCALL operator = (const VetisUser & rS);

	SString Login;
	SString Fio;
	LDATE  BirthDate;
	VetisDocument Identity;
	SString Snils;
	SString Phone;
	SString Email;
	SString WorkEmail;
	//
	int    UnionUser;
	VetisOrganization * P_Organization;
	VetisBusinessEntity * P_BusinessEntity;
	//
	SString Post;
	enum {
		fEnabled    = 0x0001,
		fNonExpired = 0x0002,
		fNonLocked  = 0x0004
	};
	long   Flags;
	TSCollection <VetisUserAuthority> AuthorityList;
};

struct VetisEnterpriseOfficialRegistration {
	VetisEnterpriseOfficialRegistration();
	~VetisEnterpriseOfficialRegistration();
	VetisEnterpriseOfficialRegistration & FASTCALL operator = (const VetisEnterpriseOfficialRegistration & rS);
	SString ID; // GRNType
	VetisBusinessEntity * P_BusinessEntity;
	SString Kpp;
};

struct VetisEnterpriseActivityList : public VetisEntityList {
	VetisEnterpriseActivityList & FASTCALL operator = (const VetisEnterpriseActivityList & rS)
	{
		TSCollection_Copy(this->Activity, rS.Activity);
		return *this;
	}
	TSCollection <VetisEnterpriseActivity> Activity;
};

struct VetisEnterprise : public VetisNamedGenericVersioningEntity {
	VetisEnterprise();
	~VetisEnterprise();
	VetisEnterprise & FASTCALL operator = (const VetisEnterprise & rS);

	SString EnglishName;
	int    Type; // EnterpriseType
	StringSet NumberList; // EnterpriseNumberList
	VetisAddress Address;
	VetisEnterpriseActivityList ActivityList;
	VetisBusinessEntity * P_Owner;
	TSCollection <VetisEnterpriseOfficialRegistration> OfficialRegistration;
};

struct VetisBusinessEntity_activityLocation {
	StringSet GlobalID; // Список GLN
	VetisEnterprise Enterprise;
};

struct VetisIncorporationForm : public VetisGenericEntity {
	VetisIncorporationForm() : VetisGenericEntity()
	{
	}
	SString Name;
	SString ShortName;
	SString Code;
};

struct VetisBusinessEntity : public VetisNamedGenericVersioningEntity {
	VetisBusinessEntity() : VetisNamedGenericVersioningEntity(), Type(0)
	{
	}
	VetisBusinessEntity & FASTCALL operator = (const VetisBusinessEntity & rS)
	{
		VetisNamedGenericVersioningEntity::operator = (rS);
		Type = rS.Type;
		IncForm = rS.IncForm;
		FullName = rS.FullName;
		Fio = rS.Fio;
		Passport = rS.Passport;
		Inn = rS.Inn;
		Kpp = rS.Kpp;
		Ogrn = rS.Ogrn;
		JuridicalAddress = rS.JuridicalAddress;
		TSCollection_Copy(ActivityLocationList, rS.ActivityLocationList);
		return *this;
	}
	int    Type; // BusinessEntityType
	VetisIncorporationForm IncForm;
	SString FullName;
	SString Fio;
	SString Passport;
	SString Inn;
	SString Kpp;
	SString Ogrn;
	VetisAddress JuridicalAddress;
	TSCollection <VetisBusinessEntity_activityLocation> ActivityLocationList;
};

struct VetisListOptions {
	VetisListOptions() : Count(0), Offset(0)
	{
	}
	uint   Count;
	uint   Offset;
};

enum VetisProductType {
	vptUndef         = 0,
	vptMeat          = 1, // 1 Мясо и мясопродукты.
	vptFeedStuff     = 2, // 2 Корма и кормовые добавки.
	vptAnimal        = 3, // 3 Живые животные.
	vptMedicine      = 4, // 4 Лекарственные средства.
	vptFood          = 5, // 5 Пищевые продукты.
	vptNonFood       = 6, // 6 Непищевые продукты и другое.
	vptFish          = 7, // 7 Рыба и морепродукты.
	vptDontReqPermit = 8 // 8 Продукция, не требующая разрешения. 
};

struct VetisProduct : public VetisNamedGenericVersioningEntity {
	VetisProduct() : VetisNamedGenericVersioningEntity(), ProductType(vptUndef)
	{
	}
	SString Code;
	SString EnglishName;
	int    ProductType; // ProductType vptXXX
};

struct VetisSubProduct : public VetisNamedGenericVersioningEntity {
	VetisSubProduct() : VetisNamedGenericVersioningEntity()
	{
	}
	SString Code;
	SString EnglishName;
	S_GUID ProductGuid;
};

struct VetisUnit : public VetisNamedGenericVersioningEntity {
	VetisUnit() : VetisNamedGenericVersioningEntity(), Factor(0)
	{
	}
	SString FullName;
	S_GUID CommonUnitGuid;
	int   Factor;
};

struct VetisPackingType : public VetisNamedGenericVersioningEntity {
	VetisPackingType() : VetisNamedGenericVersioningEntity(), GlobalID(0)
	{
	}
	int    GlobalID; // PackingCodeType
};

struct VetisPackaging {
	VetisPackaging() : Quantity(0), Volume(0.0)
	{
	}
	VetisPackingType PackagingType;
	int    Quantity;
	double Volume;
	VetisUnit Unit;
};

struct VetisProductMarks {
	VetisProductMarks() : Cls(0)
	{
	}
	SString Item;
	int   Cls; // ProductMarkingClass
};

struct VetisPackage {
	VetisPackage() : Level(0), Quantity(0)
	{
	}
	VetisPackage & FASTCALL operator = (const VetisPackage & rS)
	{
		Level = rS.Level;
		PackingType = rS.PackingType;
		Quantity = rS.Quantity;
		TSCollection_Copy(ProductMarks, rS.ProductMarks);
		return *this;
	}
	int    Level; // PackageLevelType
	VetisPackingType PackingType;
	int    Quantity;
	TSCollection <VetisProductMarks> ProductMarks;
};

struct VetisProductItem : public VetisNamedGenericVersioningEntity {
	VetisProductItem() : VetisNamedGenericVersioningEntity(), ProductType(vptUndef)
	{
	}
	VetisProductItem & FASTCALL operator = (const VetisProductItem & rS)
	{
		VetisNamedGenericVersioningEntity::operator = (rS);
		GlobalID = rS.GlobalID;
		Code = rS.Code;
		ProductType = rS.ProductType;
		Product = rS.Product;
		SubProduct = rS.SubProduct;
		Gost = rS.Gost;
		Producer = rS.Producer;
		TmOwner = rS.TmOwner;
		Packaging = rS.Packaging;
		TSCollection_Copy(Producing, rS.Producing);
		return *this;
	}
	SString GlobalID;
	SString Code;
	int    ProductType; // ProductType vptXXX
	VetisProduct Product;
	VetisSubProduct SubProduct;
	enum {
		fCorrespondsToGost = 0x00010000,
		fIsPublic          = 0x00020000
	};
	SString Gost;
	VetisBusinessEntity Producer;
	VetisBusinessEntity TmOwner;
	TSCollection <VetisEnterprise> Producing;
	VetisPackaging Packaging;
};

struct VetisGoodsDate {
	VetisGoodsDate() /*: FirstDate(ZERODATETIME), SecondDate(ZERODATETIME)*/
	{
	}
	//LDATETIME FirstDate;
	//LDATETIME SecondDate;
	SUniTime FirstDate;
	SUniTime SecondDate;
	SString InformalDate;
};

struct VetisProducer : public VetisEnterprise {
	VetisProducer() : Role(0)
	{
	}
	int    Role; // EnterpriseRole
};

struct VetisBatchOrigin {
	VetisProductItem ProductItem;
	VetisCountry Country;
	TSCollection <VetisProducer> Producer;
};

struct VetisBatch {
	VetisBatch() : ProductType(vptUndef), Volume(0.0), Flags(0), P_Owner(0)
	{
	}
	~VetisBatch()
	{
		delete P_Owner;
	}
	int    ProductType; // ProductType vptXXX
	VetisProduct Product;
	VetisSubProduct SubProduct;
	VetisProductItem ProductItem;
	double Volume;
	VetisUnit Unit;
	VetisGoodsDate DateOfProduction;
	VetisGoodsDate ExpiryDate;
	StringSet BatchIdList;
	enum {
		fPerishable    = 0x0001,
		fLowGradeCargo = 0x0002
	};
	long   Flags;
	VetisBatchOrigin Origin;
	TSCollection <VetisPackage> PackageList;
	VetisBusinessEntity * P_Owner;
};

struct VetisBusinessMember {
	VetisBusinessEntity BusinessEntity;
	VetisEnterprise Enterprise;
	SString GlobalID;
};

struct VetisCertifiedBatch : public VetisBatch {
	VetisBusinessMember Producer;
};

struct VetisTransportNumber {
	SString ContainerNumber;
	SString WagonNumber;
	SString VehicleNumber;
	SString TrailerNumber;
	SString ShipName;
	SString FlightNumber;
};

struct VetisTransportInfo {
	VetisTransportInfo() : TransportType(0)
	{
	}
	int    TransportType; // TransportType
	VetisTransportNumber TransportNumber;
};

struct VetisLocation {
	SString Name;
	SString Address;
};

struct VetisShipmentRoutePoint : public VetisGenericEntity {
	VetisShipmentRoutePoint() : UnionShipmentRoutePoint(0), Flags(0), P_Location(0), P_Enterprise(0), P_NextTransport(0)
	{
	}
	~VetisShipmentRoutePoint()
	{
		delete P_NextTransport;
		delete P_Location;
		delete P_Enterprise;
	}
	SString SqnId;
	//
	int   UnionShipmentRoutePoint;
	VetisLocation * P_Location;
	VetisEnterprise * P_Enterprise;
	//
	enum {
		fTransShipment = 0x0001
	};
	long   Flags;
	VetisTransportInfo * P_NextTransport;
};

struct VetisCertifiedConsignment {
	VetisCertifiedConsignment() : TransportStorageType(0)
	{
	}
	VetisBusinessMember Consignor;
	VetisBusinessMember Consignee;
	VetisBusinessEntity Broker;
	VetisTransportInfo TransportInfo;
	int    TransportStorageType; // TransportationStorageType
	TSCollection <VetisShipmentRoutePoint> RoutePointList;
	VetisBatch Batch;
};

struct VetisPurpose : public VetisNamedGenericVersioningEntity {
	enum {
		fForSubstandard = 0x00010000
	};
};

struct VetisVeterinaryEvent {
	VetisVeterinaryEvent() : Type(0), ActualDateTime(ZERODATETIME), UnionVeterinaryEvent(0), P_Location(0)
	{
	}
	SString ID;
	SString Name;
	int   Type; // VeterinaryEventType
	LDATETIME ActualDateTime;
	int   UnionVeterinaryEvent;
	union {
		VetisLocation * P_Location;
		VetisEnterprise * P_Enterprise;
	};
	VetisOrganization Operator;
	TSCollection <VetisDocument> ReferencedDocumentList;
	SString Notes;
};

typedef VetisNamedGenericVersioningEntity VetisIndicator;
typedef VetisNamedGenericVersioningEntity VetisAnimalDisease;
typedef VetisNamedGenericVersioningEntity VetisResearchMethod;

struct VetisLaboratoryResearchEvent : public VetisVeterinaryEvent {
	VetisLaboratoryResearchEvent() : VetisVeterinaryEvent(), Union_LaboratoryResearchEvent(0), Result(0)
	{
	}
	StringSet BatchIdList;
	SString ExpertiseID;
	int   Union_LaboratoryResearchEvent;
	VetisNamedGenericVersioningEntity IndOrDis; // VetisIndicator || VetisAnimalDisease
	VetisResearchMethod Method;
	int    Result; // ResearchResult
	SString Conclusion;
};

struct VetisQuarantineEvent : public VetisVeterinaryEvent {
	SString Duration;
};

struct VetisMedicinalDrug {
	SString ID;
	SString Name;
	SString Series;
	VetisBusinessMember Producer;
};

struct VetisAnimalMedicationEvent : public VetisVeterinaryEvent {
	VetisAnimalMedicationEvent() : VetisVeterinaryEvent(), EffectiveBeforeDate(ZERODATETIME)
	{
	}
	VetisAnimalDisease Disease;
	VetisMedicinalDrug MedicinalDrug;
	LDATETIME EffectiveBeforeDate;
};

struct VetisRegionalizationCondition : public VetisGenericVersioningEntity {
	VetisRegionalizationCondition() : VetisGenericVersioningEntity()
	{
	}
	enum {
		fStrict = 0x00010000
	};
	SString ReferenceNumber;
	SString Text;
	TSCollection <VetisAnimalDisease> RelatedDiseaseList;
};

struct VetisRegionalizationClause {
	VetisRegionalizationCondition Condition;
	SString Text;
};

struct VetisVeterinaryAuthentication {
	VetisVeterinaryAuthentication() : Flags(0), CargoExpertized(0), AnimalSpentPeriod(0)
	{
	}
	VetisPurpose Purpose;
	enum {
		fCargoInspected = 0x0001
	};
	long   Flags;
	int    CargoExpertized; // ResearchResult
	int    AnimalSpentPeriod; // AnimalSpentPeriod
	SString LocationProsperity; 
	SString MonthsSpent;
	SString SpecialMarks;
	TSCollection <VetisLaboratoryResearchEvent> LaboratoryResearchList;
	TSCollection <VetisAnimalMedicationEvent> ImmunizationList;
	TSCollection <VetisVeterinaryEvent> VeterinaryEventList;
	TSCollection <VetisRegionalizationClause> R13nClauseList;
};

struct VetisReferencedDocument : public VetisDocument {
	VetisReferencedDocument() : VetisDocument(), RelationshipType(0)
	{
	}
	int    RelationshipType; // ReferenceType
};

struct VetisVetDocumentStatusChange {
	VetisVetDocumentStatusChange() : Status(0), ActualDateTime(ZERODATETIME)
	{
	}
	int    Status; // VetDocumentStatus
	VetisUser SpecifiedPerson;
	LDATETIME ActualDateTime;
	SString Reason;
};

struct VetisVetDocument : public VetisDocument {
	VetisVetDocument() : VetisDocument(), VetDForm(0), VetDType(0), VetDStatus(0), Flags(0), LastUpdateDate(ZERODATETIME),
		UnionVetDocument(0), P_CertifiedBatch(0)
	{
	}
	~VetisVetDocument()
	{
		delete P_CertifiedBatch;
	}
	enum Form {
		formCERTCU1 = 0, 
		formLIC1 = 1,
		formCERTCU2 = 2, 
		formLIC2 = 3, 
		formCERTCU3 = 4, 
		formLIC3 = 5, 
		formNOTE4 = 6, 
		formCERT5I = 7, 
		formCERT61 = 8, 
		formCERT62 = 9, 
		formCERT63 = 10, 
		formPRODUCTIVE = 11
	};
	int    VetDForm;   // VetisVetDocument::formXXX
	int    VetDType;   // VetDocumentType
	int    VetDStatus; // VetDocumentStatus
	enum {
		fFinalized = 0x00010000
	};
	long   Flags;
	LDATETIME LastUpdateDate;
	//
	int    UnionVetDocument;
	VetisCertifiedBatch * P_CertifiedBatch;
	VetisCertifiedConsignment CertifiedConsignment;
	//
	VetisVeterinaryAuthentication Authentication;
	SString PrecedingVetDocuments;
	TSCollection <VetisReferencedDocument> ReferencedDocumentList;
	TSCollection <VetisVetDocumentStatusChange> StatusChangeList;
};

struct VetisStockEntryEventList {
	TSCollection <VetisLaboratoryResearchEvent> LaboratoryResearchList;
	TSCollection <VetisQuarantineEvent> QuarantineList;
	TSCollection <VetisAnimalMedicationEvent> ImmunizationList;
	TSCollection <VetisVeterinaryEvent> VeterinaryEventList;
};

struct VetisStockEntry : public VetisGenericVersioningEntity {
	SString EntryNumber;
	VetisBatch Batch;
	VetisStockEntryEventList VetEventList;
	TSCollection <VetisVetDocument> VetDocumentList;
};

struct VetisStockEntrySearchPattern : public VetisStockEntry {
	VetisStockEntrySearchPattern() : VetisStockEntry(), BlankFilter(0)
	{
	}
	int    BlankFilter; // StockEntryBlankFilter
};

class VetisApplicationData {
public:
	enum {
		signNone = 0,
		signGetStockEntryList,
		signGetBusinessEntity,
		signGetBusinessEntityByGuid,
		signGetAppliedUserAuthorityList,
		signGetRussianEnterpriseList,
		signGetVetDocumentList,
		signGetProductItemList
	};
	VetisApplicationData(long sign) : Sign(sign)
	{
	}
	virtual ~VetisApplicationData()
	{
	}
	//
	const long Sign;
	SString LocalTransactionId;
	VetisUser Initiator;
	SString SessionToken;
};

class VetisGetStockEntryListRequest : public VetisApplicationData {
public:
	VetisGetStockEntryListRequest() : VetisApplicationData(signGetStockEntryList)
	{
	}
	VetisListOptions ListOptions;
	VetisStockEntrySearchPattern SearchPattern;
};

class VetisGetVetDocumentListRequest : public VetisApplicationData {
public:
	VetisGetVetDocumentListRequest() : VetisApplicationData(signGetVetDocumentList), DocType(-1), DocStatus(-1)
	{
	}
	VetisListOptions ListOptions;
	int    DocType;   // VetisDocType
	int    DocStatus; // VetisDocStatus
};

class VetisListOptionsRequest : public VetisApplicationData {
public:
	VetisListOptionsRequest(long sign) : VetisApplicationData(sign)
	{
	}
	VetisListOptions ListOptions;
};

class VetisGetBusinessEntityRequest : public VetisApplicationData {
public:
	VetisGetBusinessEntityRequest() : VetisApplicationData(signGetBusinessEntity)
	{
	}
};

struct VetisApplicationBlock {
	explicit VetisApplicationBlock(const VetisApplicationData * pAppParam);
	VetisApplicationBlock(const VetisApplicationBlock & rS);
	~VetisApplicationBlock();
	VetisApplicationBlock & FASTCALL operator = (const VetisApplicationBlock & rS);
	void   Clear();
	int    FASTCALL Copy(const VetisApplicationBlock & rS);

	enum {
		appstUndef     = -1,
		appstAccepted  = 0,
		appstInProcess = 1,
		appstCompleted = 2,
		appstRejected  = 3
	};
	enum {
		opUndef = 0,
		opGetVetDocumentList
	};
	int    ApplicationStatus;
	int64  LocalTransactionId;
	SString ServiceId;
	SString User;
	S_GUID ApplicationId;
	S_GUID IssuerId;
	S_GUID EnterpriseId;
	LDATETIME IssueDate;
	LDATETIME RcvDate;
	LDATETIME PrdcRsltDate;
	SString AppData; // xml-block

	TSCollection <VetisErrorEntry> ErrList;
	TSCollection <VetisFault> FaultList;
	const VetisApplicationData * P_AppParam;
};
//
//
//
VetisEnterpriseOfficialRegistration::VetisEnterpriseOfficialRegistration() : P_BusinessEntity(0)
{
}
VetisEnterpriseOfficialRegistration::~VetisEnterpriseOfficialRegistration()
{
	delete P_BusinessEntity;
}
VetisEnterpriseOfficialRegistration & FASTCALL VetisEnterpriseOfficialRegistration::operator = (const VetisEnterpriseOfficialRegistration & rS)
{
	ID = rS.ID;
	Kpp = rS.Kpp;
	ZDELETE(P_BusinessEntity);
	if(rS.P_BusinessEntity) {
		P_BusinessEntity = new VetisBusinessEntity;
		ASSIGN_PTR(P_BusinessEntity, *rS.P_BusinessEntity);
	}
	return *this;
}

VetisUser::VetisUser() : BirthDate(ZERODATE), Flags(0), UnionUser(0), P_Organization(0), P_BusinessEntity(0)
{
}

VetisUser::~VetisUser()
{
	delete P_Organization;
	delete P_BusinessEntity;
}

VetisUser & FASTCALL VetisUser::operator = (const VetisUser & rS)
{
	Login = rS.Login;
	Fio = rS.Fio;
	BirthDate = rS.BirthDate;
	Identity = rS.Identity; // !
	Snils = rS.Snils;
	Phone = rS.Phone;
	Email = rS.Email;
	WorkEmail = rS.WorkEmail;
	UnionUser = rS.UnionUser;
	Post = rS.Post;
	Flags = rS.Flags;
	TSCollection_Copy(AuthorityList, rS.AuthorityList);
	ZDELETE(P_Organization);
	ZDELETE(P_BusinessEntity);
	if(rS.P_Organization) {
		P_Organization = new VetisOrganization;
		ASSIGN_PTR(P_Organization, *rS.P_Organization);
	}
	if(rS.P_BusinessEntity) {
		P_BusinessEntity = new VetisBusinessEntity;
		ASSIGN_PTR(P_BusinessEntity, *rS.P_BusinessEntity);
	}
	return *this;
}

VetisEnterprise::VetisEnterprise() : P_Owner(0), Type(0)
{
}

VetisEnterprise::~VetisEnterprise()
{
	delete P_Owner;
}

VetisEnterprise & FASTCALL VetisEnterprise::operator = (const VetisEnterprise & rS)
{
	VetisNamedGenericVersioningEntity::operator = (rS);
	EnglishName = rS.EnglishName;
	Type = rS.Type;
	NumberList = rS.NumberList;
	Address = rS.Address;
	ActivityList = rS.ActivityList;
	ZDELETE(P_Owner);
	if(rS.P_Owner) {
		P_Owner = new VetisBusinessEntity;
		ASSIGN_PTR(P_Owner, *rS.P_Owner);
	}
	TSCollection_Copy(OfficialRegistration, rS.OfficialRegistration);
	return *this;
}

VetisApplicationBlock::VetisApplicationBlock(const VetisApplicationData * pAppParam) : ApplicationStatus(appstUndef), 
	IssueDate(ZERODATETIME), RcvDate(ZERODATETIME), PrdcRsltDate(ZERODATETIME), P_AppParam(pAppParam),
	LocalTransactionId(0)//, P_GselReq(0), P_LoReq(0), P_Ent(0), P_GvdlReq(0)
{
}

VetisApplicationBlock::VetisApplicationBlock(const VetisApplicationBlock & rS) //: P_GselReq(0)
{
	Copy(rS);
}

VetisApplicationBlock::~VetisApplicationBlock()
{
}

VetisApplicationBlock & FASTCALL VetisApplicationBlock::operator = (const VetisApplicationBlock & rS)
{
	Copy(rS);
	return *this;
}

void VetisApplicationBlock::Clear()
{
	ApplicationStatus = appstUndef;
	LocalTransactionId = 0;
	ServiceId.Z();
	User.Z();
	ApplicationId.Z();
	IssuerId.Z();
	EnterpriseId.Z();
	IssueDate.Z();
	RcvDate.Z();
	PrdcRsltDate.Z();
	ErrList.freeAll();
	FaultList.freeAll();
	AppData.Z();
}

int FASTCALL VetisApplicationBlock::Copy(const VetisApplicationBlock & rS)
{
	int    ok = 1;
	ApplicationStatus = rS.ApplicationStatus;
	LocalTransactionId = rS.LocalTransactionId;
	ServiceId = rS.ServiceId;
	User = rS.User;
	ApplicationId = rS.ApplicationId;
	IssuerId = rS.IssuerId;
	EnterpriseId = rS.EnterpriseId;
	IssueDate = rS.IssueDate;
	RcvDate = rS.RcvDate;
	PrdcRsltDate = rS.PrdcRsltDate;
	TSCollection_Copy(ErrList, rS.ErrList);
	TSCollection_Copy(FaultList, rS.FaultList);
	AppData = rS.AppData;
	P_AppParam = rS.P_AppParam;
	return ok;
}

//typedef VetisApplicationBlock * (* VETIS_SUBMITAPPLICATIONREQUEST_PROC)(PPSoapClientSession & rSess, const char * pApiKey, const VetisApplicationBlock & rBlk);
//typedef VetisApplicationBlock * (* VETIS_RECEIVEAPPLICATIONRESULT_PROC)(PPSoapClientSession & rSess, const char * pApiKey, const S_GUID & rIssuerId, const S_GUID & rApplicationId);
//typedef TSCollection <VetisEnterprise> * (* VETIS_GETRUSSIANENTERPRISELIST_PROC)(PPSoapClientSession & rSess, VetisListOptionsRequest * pListReq, VetisEnterprise * pEntFilt);
//typedef TSCollection <VetisProductItem> * (* VETIS_GETPRODUCTITEMLIST_PROC)(PPSoapClientSession & rSess, VetisListOptionsRequest * pListReq, VetisEnterprise * pEntFilt);

class SXmlWriter {
public:
	SXmlWriter()
	{
		P_XmlBuf = xmlBufferCreate();
		P_Writer = xmlNewTextWriterMemory(P_XmlBuf, 0);
		xmlTextWriterSetIndent(P_Writer, 1);
	}
	~SXmlWriter()
	{
		xmlFreeTextWriter(P_Writer);
		xmlBufferFree(P_XmlBuf);
	}
	operator xmlTextWriter * () const { return P_Writer; }
	operator xmlBuffer * () const { return P_XmlBuf; }
private:
	xmlBuffer * P_XmlBuf;
	xmlTextWriter * P_Writer;
};

class VetisProductCore {
public:
	enum {
		kProductItem = 1,
		kProduct     = 2,
		kSubProduct  = 3
	};
	SLAPI  VetisProductCore()
	{
	}
    int    SLAPI Put(PPID * pID, int kind, VetisProductItem * pItem, int use_ta)
	{
		int    ok = -1;
		Reference * p_ref = PPRef;
		SString temp_buf;
		VetisProductTbl::Rec rec;
		/*
				table VetisProduct {
					autolong ID;
					long   GuidRef;  // ->UuidRef.ID Ссылка на GUID
					long   UuidRef;  // ->UuidRef.ID Ссылка на UUID
					long   Kind;     // Вид записи (1 - product item, 2 - product, 3 - subproduct)
					long   ProductType;
					long   Status;
					long   ProductID;    // для ProductItem
					long   SubProductID; // для ProductItem
					long   ProducerLocID;
					long   Flags;
					raw    Reserve[128];
				index:
					ID (unique);
					GuidRef, UuidRef (unique);
					UuidRef, GuidRef (unique);
					ProductID (dup mod anysegnull);
					SubProductID (dup mod anysegnull);
				file:
					"vetisprd.btr";
					access=1;
				}
		*/
		{
			MEMSZERO(rec);
			PPID   guid_ref;
			PPID   uuid_ref;
			int gr = UrT.GetUuid(pItem->Guid, &guid_ref, 0, 0);
			THROW(gr);
			int ur = UrT.GetUuid(pItem->Uuid, &uuid_ref, 0, 0);
			rec.Kind = kind;
			rec.ProductType = pItem->ProductType;
			rec.Status = pItem->Status;
			if(kind == kProductItem) {
				PPID   product_guid_ref = 0;
				PPID   product_uuid_ref = 0;
				PPID   subproduct_guid_ref = 0;
				PPID   subproduct_uuid_ref = 0;
				int pgr = UrT.GetUuid(pItem->Product.Guid, &product_guid_ref, 0, 0);
				int pur = UrT.GetUuid(pItem->Product.Uuid, &product_uuid_ref, 0, 0);
				int sgr = UrT.GetUuid(pItem->SubProduct.Guid, &subproduct_guid_ref, 0, 0);
				int sur = UrT.GetUuid(pItem->SubProduct.Uuid, &subproduct_uuid_ref, 0, 0);
			}
		}
		{
			PPTransaction tra(use_ta);
			THROW(tra);
			{
			}
			if(*pID) {
			}
			else {
			}
			THROW(tra.Commit());
		}
		CATCHZOK
		return ok;
	}
	int    SLAPI Put(PPID * pID, VetisProduct * pItem, int use_ta);
	int    SLAPI Put(PPID * pID, VetisSubProduct * pItem, int use_ta);
	int    SLAPI RecToItem(const VetisProductTbl::Rec & rRec, VetisProductItem & rItem)
	{
		int    ok = -1;
		return ok;
	}
	int    SLAPI RecToItem(const VetisProductTbl::Rec & rRec, VetisProduct & rItem);
	int    SLAPI RecToItem(const VetisProductTbl::Rec & rRec, VetisSubProduct & rItem);
    int    SLAPI Search(PPID id, VetisProductItem & rItem);
    int    SLAPI SearchByGuid(const S_GUID & rU, VetisProductItem & rItem);
	int    SLAPI SearchByUuid(const S_GUID & rU, VetisProductItem & rItem);
    int    SLAPI Export(long fmt, const char * pFileName);

	VetisProductTbl PiT;
	UuidRefCore UrT;
};

class PPVetisInterface {
public:
	enum {
		extssApplicationId = 1,
		extssServiceId,
		extssApiKey,
		extssUser,
		extssPassword,
		extssQInitiator // логин инициатора запросов
	};
	enum {
		stInited = 0x0001
	};
	struct Param : public PPExtStrContainer {
		SLAPI  Param() 
		{
		}
		void   Clear()
		{
			SetBuffer(0);
			IssuerUUID.Z();
			EntUUID.Z();
		}
		S_GUID IssuerUUID; // GUID пользователя, генерурующего запросы
		S_GUID EntUUID; // GUID предприятия
	};
	SLAPI  PPVetisInterface();
	SLAPI ~PPVetisInterface();
	int    SLAPI Init(const Param & rP);

	int    SLAPI GetStockEntryList(uint startOffset, uint count);
	//
	// Операция GetVetDocumentListOperation предназначена для получения всех ветеринарно-сопроводительных документов предприятия. 
	//   При этом список ВСД может быть отфильтрован по следующим критериям:
	//     Тип ВСД: входящий ВСД; исходящий ВСД; производственный ВСД; транспортный ВСД; возвратный ВСД. 
	//     Статус ВСД: оформлен; погашен; аннулирован. 
	//   На вход системы передаются следующие сведения:
	//     информация о пользователе - инициаторе запроса;
	//     информация о предприятии и хозяйствующем субъекте, где осуществляется поиск ВСД;
	//     параметры, по которым будет отфильтрован список ВСД. 
	//   Результатом выполнения данной операции является:
	//     пользователю передаются сведения о запрашиваемых ВСД. 
	//   Запрашиваться пользователем могут только те ВСД, где ветеринарное управление инициатор запроса обслуживает предприятия. 
	//
	int    SLAPI GetVetDocumentList(uint startOffset, uint count);
	//
	// Операция IncomingOperation предназначена для оформления в системе Меркурий входящей партии. 
	//   На вход системы, в зависимости от сценария, передаются следующие сведения:
	//     информация об электронном ВСД, по которому продукция поступила на предприятие (для сценария №1);
	//     информация о бумажном ВСД, по которому продукция поступила на предприятие (для сценария №2);
	//     фактические сведения о принимаемой партии;
	//     акт несоответствия, в случае если фактические сведения о продукции отличаются от сведений, указанных в ВСД;
	//     возвратный ВСД, в случае если на весь объем или на его часть оформляется возврат. 
	//   Результатом выполнения данной операции является:
	//     оформление электронного ВСД, в случае, если продукция поступила по бумажному входящему документу (для сценария №2);
	//     гашение электронного ВСД (для сценария №1);
	//     добавление одной записи в журнал входящей продукции;
	//     возвратный ВСД (формируется в случае, если принимается не весь объем продукции);
	//     акт несоответствия (формируется в случае, если фактические сведения о продукции не совпадают с указанными в ВСД). 
	//
	int    SLAPI ProcessIncomingConsignment(); // processIncomingConsignmentRequest
	//
	// Операция PrepareOutgoingConsignmentOperation предназначена для оформления в системе Меркурий транспортной партии. 
	//   На вход системы передаются следующие сведения:
	//     информация об одной или нескольких партиях продукции, из которых будет сформирована транспортная партия;
	//     сведения о получателе транспортной партии;
	//     сведения о транспортном средстве и маршруте его следования;
	//     дополнительные сведения необходимые для оформления ветеринарно-сопроводительного документа (ВСД), например, 
	//     результат ветеринарно-санитарной экспертизы, сведения о ТТН, особые отметки и т.д. 
	//   Результатом выполнения данной операции является:
	//     списание объема с одной или нескольких записей журнала продукции, которые были указаны в заявке;
	//     гашение производственной сертификата, если был указан весь объем по данной записи журнала вырабатываемой продукции;
	//     для каждого наименования продукции указанного в транспортной партии, система Меркурий формирует ветеринарно-сопроводительный документ (ВСД). 
	//
	int    SLAPI PrepareOutgoingConsignment(); // prepareOutgoingConsignmentRequest
	//
	// Операция RegisterProductionOperation предназначена для оформления в системе Меркурий производственной партии, как завершённой, так и незавершённой. 
	//   На вход системы передаются следующие сведения:
	//     информация о сырье, из которого партия или несколько партий были произведены;
	//     информация о произведенной партии или нескольких партиях продукции;
	//     информация о хозяйствующем субъекте - собственнике сырья и выпускаемой продукции и информация о площадке, на которой продукция выпускается;
	//     идентификатор производственной операции (для незавершённого производства);
	//     номер производственной партии;
	//     флаг завершения производственной транзакции. 
	//   Результатом выполнения данной операции является:
	//     списание объема с одной или нескольких записей журнала продукции, указанного в качестве сырья;
	//     добавление одной или нескольких записей в журнал вырабатываемой продукции о партии продукции, которая была произведена или присоединение к 
	//       существующей записи вырабатываемой продукции, если оформляется незаверёшнное производство;
	//     для каждой записи журнала вырабатываемой продукции, которая была добавлена при выполнении операции, система Меркурий формирует 
	//       ветеринарно-сопроводительный документ (ВСД) или происходит увеличение объёма выпущенной продукции в уже оформленном ветеринарном документе 
	//       (для незавершённого производства). 
	//
	int    SLAPI RegisterProductionOperation(); // registerProductionOperationRequest
	//
	// Операция AddBussinessEntityUser предназначена для регистрации новых пользователей в системе Меркурий или 
	//   привязки существующих пользователей к хозяйствующему субъекту.
	//   При выполнении операции на вход системы передаются следующие сведения:
	//     информация о пользователе - инициаторе запроса;
	//     имя пользователя или уникальный идентификатор, если существующий пользователь привязывается к ХС;
	//     данные пользователя (ФИО, паспортные данные, гражданство, адрес электронной почты), если регистрируется новый пользователь;
	//     при регистрации нового пользователя опционально могут быть переданы дополнительные данные пользователя (телефон, рабочий телефон, рабочий адрес электронной почты и т.д.), которые будут сохранены в системе "Ветис.Паспорт";
	//     при регистрации нового пользователя опционально может быть передан список прав пользователя, но назначены эти права будут после активации созданного пользователя. 
	//   Результатом выполнения данной операции является:
	//     регистрация нового пользователя или привязка существующего пользователя к хозяйствующему субъекту. 
	//
	int    SLAPI AddBusinessEntityUser(); // addBusinessEntityUserRequest
	int    SLAPI GetAppliedUserAuthorityList();

	int    SLAPI GetRussianEnterpriseList(uint offs, uint count, TSCollection <VetisEnterprise> & rResult);
	int    SLAPI GetProductItemList(uint offs, uint count, TSCollection <VetisProductItem> & rResult);

	enum {
		qtProductItemByGuid = 1,
		qtProductItemByUuid,
		qtProductByGuid,
		qtProductByUuid,
		qtSubProductByGuid,
		qtSubProductByUuid
	};

	int    SLAPI GetProductQuery(int queryType, const char * pQueryParam, VetisProductItem & rResult);

	static int SLAPI SetupParam(Param & rP);
private:
	class VetisSubmitRequestBlock : public SXmlWriter {
	public:
		VetisSubmitRequestBlock();
		~VetisSubmitRequestBlock();
	private:
		SXml::WDoc D;
	};
	int    SLAPI SubmitRequest(VetisApplicationBlock & rAppBlk, VetisApplicationBlock & rResult);
	int    ReceiveResult(const S_GUID & rAppId, VetisApplicationBlock & rResult);
	//int    SLAPI PrepareAppReqData(VetisApplicationBlock & rBlk, const void * pAppData);
	int    SLAPI PreprocessResult(const void * pResult, const PPSoapClientSession & rSess);
	void   FASTCALL DestroyResult(void ** ppResult);
	int    SLAPI SendSOAP(const char * pUrl, const char * pAction, const SString & rPack, SString & rReply);
	int    SLAPI MakeAuthField(SString & rBuf);
	int    SLAPI ParseReply(const SString & rReply, VetisApplicationBlock & rResult);
	int    SLAPI ParseError(xmlNode * pNode, VetisErrorEntry & rResult);
	int    SLAPI ParseFault(xmlNode * pParentNode, VetisFault & rResult);
	int    SLAPI ParseApplicationStatus(xmlNode * pNode, int & rStatus);
	int    SLAPI ParseApplicationBlock(xmlNode * pParentNode, VetisApplicationBlock & rResult);
	int    SLAPI ParseVetDocument(xmlNode * pParentNode, VetisVetDocument & rResult);
	int    SLAPI ParseCertifiedConsignment(xmlNode * pParentNode, VetisCertifiedConsignment & rResult);
	int    SLAPI ParseBusinessMember(xmlNode * pParentNode, VetisBusinessMember & rResult);
	int    SLAPI ParseEnterprise(xmlNode * pParentNode, VetisEnterprise & rResult);
	int    SLAPI ParseBusinessEntity(xmlNode * pParentNode, VetisBusinessEntity & rResult);
	int    SLAPI ParseNamedGenericVersioningEntity(xmlNode * pParentNode, VetisNamedGenericVersioningEntity & rResult);
	int    SLAPI ParseBatch(xmlNode * pParentNode, VetisBatch & rResult);
	int    SLAPI ParseProduct(xmlNode * pParentNode, VetisProduct & rResult);
	int    SLAPI ParseSubProduct(xmlNode * pParentNode, VetisSubProduct & rResult);
	int    SLAPI ParseProductItem(xmlNode * pParentNode, VetisProductItem & rResult);
	int    SLAPI ParseComplexDate(xmlNode * pParentNode, SUniTime & rResult);
	int    SLAPI ParseGoodsDate(xmlNode * pParentNode, VetisGoodsDate & rResult);

	long   State;
	SString LogFileName;
	SString LastMsg;
	SDynLibrary * P_Lib;
	void * P_DestroyFunc;
	Param   P;
	int64   LastLocalTransactionId;
};

//static 
int SLAPI PPVetisInterface::SetupParam(Param & rP)
{
	rP.Clear();

	int    ok = 1;
	Reference * p_ref = PPRef;
	PPID   main_org_id = 0;
	SString temp_buf;
	PPAlbatrosConfig acfg;
	THROW(PPAlbatrosCfgMngr::Get(&acfg) > 0);
	GetMainOrgID(&main_org_id);
	acfg.GetExtStrData(ALBATROSEXSTR_VETISUSER, temp_buf);
	THROW(temp_buf.NotEmptyS()); // @error
	rP.PutExtStrData(extssUser, temp_buf);
	acfg.GetPassword(ALBATROSEXSTR_VETISPASSW, temp_buf);
	THROW(temp_buf.NotEmptyS()); // @error
	rP.PutExtStrData(extssPassword, temp_buf);
	acfg.GetExtStrData(ALBATROSEXSTR_VETISAPIKEY, temp_buf);
	THROW(temp_buf.NotEmptyS()); // @error
	rP.PutExtStrData(extssApiKey, temp_buf);
	{
		ObjTagItem tag_item;
		if(p_ref->Ot.GetTag(PPOBJ_PERSON, main_org_id, PPTAG_PERSON_VETISUUID, &tag_item) > 0) {
			tag_item.GetGuid(&rP.EntUUID);
		}
		//
		if(p_ref->Ot.GetTag(PPOBJ_PERSON, main_org_id, PPTAG_PERSON_VETISISSUUID, &tag_item) > 0) {
			tag_item.GetGuid(&rP.IssuerUUID);
		}
		THROW(!rP.IssuerUUID.IsZero()); // @error
		//
		if(p_ref->Ot.GetTag(PPOBJ_PERSON, main_org_id, PPTAG_PERSON_VETISUSER, &tag_item) > 0) {
			tag_item.GetStr(temp_buf);
			rP.PutExtStrData(extssQInitiator, temp_buf);
		}
	}
	CATCHZOK
	return ok;
}

SLAPI PPVetisInterface::PPVetisInterface() : State(0), P_Lib(0), P_DestroyFunc(0), LastLocalTransactionId(0)
{
	PPGetFilePath(PPPATH_LOG, "vetis.log", LogFileName);
 	{
		SString lib_path;
		PPGetFilePath(PPPATH_BIN, "PPSoapMercury_AMS.dll", lib_path);
		P_Lib = new SDynLibrary(lib_path);
		if(P_Lib && !P_Lib->IsValid()) {
			ZDELETE(P_Lib);
		}
		if(P_Lib)
			P_DestroyFunc = (void *)P_Lib->GetProcAddr("VetisDestroyResult");
	}
}

SLAPI PPVetisInterface::~PPVetisInterface()
{
	P_DestroyFunc = 0;
	delete P_Lib;
}

int SLAPI PPVetisInterface::Init(const Param & rP)
{
	int    ok = 1;
	P = rP;
	State |= stInited;
	return ok;
}

int SLAPI PPVetisInterface::PreprocessResult(const void * pResult, const PPSoapClientSession & rSess)
{
	LastMsg = rSess.GetMsg();
    return BIN(pResult);
}

void FASTCALL PPVetisInterface::DestroyResult(void ** ppResult)
{
	if(P_DestroyFunc) {
		((UHTT_DESTROYRESULT)P_DestroyFunc)(*ppResult);
		*ppResult = 0;
	}
}

#if 0 // {
int SLAPI PPVetisInterface::PrepareAppReqData(VetisApplicationBlock & rBlk, const void * pAppData)
{
	int    ok = 0;
	if(rBlk.Func == VetisApplicationData::signGetStockEntryListRequest) {
		rBlk.P_GselReq = new VetisGetStockEntryListRequest;
		rBlk.P_GselReq->Initiator.Login = rBlk.User;
		rBlk.P_GselReq->ListOptions.Count = 20;
		ok = 1;
	}
	else if(rBlk.Func == VetisApplicationData::signGetAppliedUserAuthorityListRequest) {
		rBlk.P_LoReq = new VetisListOptionsRequest;
		rBlk.P_LoReq->Initiator.Login = rBlk.User;
		rBlk.P_LoReq->ListOptions.Count = 20;
		ok = 1;
	}
	else if(rBlk.Func == VetisApplicationData::signGetVetDocumentListRequest) {
		rBlk.P_GvdlReq = new VetisGetVetDocumentListRequest;
		rBlk.P_GvdlReq->Initiator.Login = rBlk.User;
		rBlk.P_GvdlReq->ListOptions.Count = 20;
		rBlk.P_GvdlReq->DocType = vetisdoctypINCOMING;
		rBlk.P_GvdlReq->DocStatus = vetisdocstCREATED;
		ok = 1;
	}
	return ok;
}
#endif // } 0

static const char * P_VetisSoapUrl = "https://api.vetrf.ru/platform/services/ApplicationManagementService"; // product
//static const char * P_VetisSoapUrl = "https://api2.vetrf.ru:8002/platform/services/ApplicationManagementService"; // test

int SLAPI PPVetisInterface::MakeAuthField(SString & rBuf)
{
	int    ok = 1;
	rBuf.Z();
	SString pwd;
	SString login;
	SString temp_buf;
	P.GetExtStrData(extssUser, login);
	P.GetExtStrData(extssPassword, pwd);
	login.CatChar(':').Cat(pwd).Transf(CTRANSF_INNER_TO_UTF8);
	temp_buf.Z().EncodeMime64(login.cptr(), login.Len());
	rBuf.Cat("Basic").Space().Cat(temp_buf);
	pwd.Obfuscate();
	login.Obfuscate();
	return ok;
}

int SLAPI PPVetisInterface::SendSOAP(const char * pUrl, const char * pAction, const SString & rPack, SString & rReply)
{
	rReply.Z();
	int    ok = -1;
	SString temp_buf;
	ScURL  c;
	InetUrl url(NZOR(pUrl, P_VetisSoapUrl));
	StrStrAssocArray hdr_flds;
	SBuffer ack_buf;
	SFile wr_stream(ack_buf, SFile::mWrite);
	{
		//Content-Type: text/xml; charset=utf-8
		SFileFormat::GetMime(SFileFormat::Xml, temp_buf);
		temp_buf.CatDiv(';', 2).CatEq("charset", "utf-8");
		SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrContentType, temp_buf);
		SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrContentLen, temp_buf.Z().Cat(rPack.Len()));
		THROW(MakeAuthField(temp_buf));
		SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrAuthorization, temp_buf);
		if(!isempty(pAction)) {
			//SOAPAction: "receiveApplicationResult"
			SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrSoapAction, temp_buf.CatQStr(pAction));
		}
	}
	THROW_SL(c.HttpPost(url, ScURL::mfDontVerifySslPeer|ScURL::mfVerbose, &hdr_flds, rPack, &wr_stream));
	{
		SBuffer * p_ack_buf = (SBuffer *)wr_stream;
		if(p_ack_buf) {
			const int avl_size = (int)p_ack_buf->GetAvailableSize();
			rReply.Z().CatN((const char *)p_ack_buf->GetBuf(), avl_size);
			//f_out_test.WriteLine(temp_buf.CR().CR());
		}
	}
	CATCHZOK
	return ok;
}

PPVetisInterface::VetisSubmitRequestBlock::VetisSubmitRequestBlock() : SXmlWriter(), D((xmlTextWriter *)*this, cpUTF8)
{
}

PPVetisInterface::VetisSubmitRequestBlock::~VetisSubmitRequestBlock()
{
}

int SLAPI PPVetisInterface::ParseError(xmlNode * pNode, VetisErrorEntry & rResult)
{
	int    ok = -1;
	SString temp_buf;
	rResult.Z();
	if(SXml::GetContentByName(pNode, "error", temp_buf)) {
		rResult.Item = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		if(SXml::GetAttrib(pNode, "code", temp_buf) > 0)
			rResult.Code = temp_buf;
		ok = 1;
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseFault(xmlNode * pParentNode, VetisFault & rResult)
{
	int    ok = 1;
	if(pParentNode) {
		SString temp_buf;
		VetisErrorEntry err;
		for(xmlNode * p_i2 = pParentNode->children; p_i2; p_i2 = p_i2->next) {
			if(SXml::GetContentByName(p_i2, "message", temp_buf)) {
				rResult.Message = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
			}
			else if(ParseError(p_i2, err) > 0) {
				VetisErrorEntry * p_err = rResult.ErrList.CreateNewItem();
				THROW_SL(p_err);
				*p_err = err;
			}
		}
	}
	CATCHZOK
	return ok;
}

static const SIntToSymbTabEntry VetisAppStatus_SymbTab[] = {
	{ VetisApplicationBlock::appstAccepted, "Accepted" },
	{ VetisApplicationBlock::appstRejected, "Rejected" },
	{ VetisApplicationBlock::appstCompleted, "Completed" },
	{ VetisApplicationBlock::appstInProcess, "InProcess" }
};

int SLAPI PPVetisInterface::ParseApplicationStatus(xmlNode * pNode, int & rStatus)
{
	int    ok = -1;
	SString temp_buf;
	if(SXml::GetContentByName(pNode, "status", temp_buf)) {
		rStatus = SIntToSymbTab_GetId(VetisAppStatus_SymbTab, SIZEOFARRAY(VetisAppStatus_SymbTab), temp_buf);
		ok = 1;
	}
	return ok;
}

static const SIntToSymbTabEntry VetisVetDocFormat_SymbTab[] = {
	{ VetisVetDocument::formCERTCU1, "CERTCU1" },
	{ VetisVetDocument::formLIC1, "LIC1" },
	{ VetisVetDocument::formCERTCU2, "CERTCU2" },
	{ VetisVetDocument::formLIC2, "LIC2" },
	{ VetisVetDocument::formCERTCU3, "CERTCU3" },
	{ VetisVetDocument::formLIC3, "LIC3" },
	{ VetisVetDocument::formNOTE4, "NOTE4" },
	{ VetisVetDocument::formCERT5I, "CERT5I" },
	{ VetisVetDocument::formCERT61, "CERT61" },
	{ VetisVetDocument::formCERT62, "CERT62" },
	{ VetisVetDocument::formCERT63, "CERT63" },
	{ VetisVetDocument::formPRODUCTIVE, "PRODUCTIVE" }
};

static const SIntToSymbTabEntry VetisVetDocType_SymbTab[] = {
	{ vetisdoctypTRANSPORT, "TRANSPORT" },
	{ vetisdoctypPRODUCTIVE, "PRODUCTIVE" },
	{ vetisdoctypRETURNABLE, "RETURNABLE" },
	{ vetisdoctypINCOMING, "INCOMING" },
	{ vetisdoctypOUTGOING, "OUTGOING" }
};

static const SIntToSymbTabEntry VetisVetDocStatus_SymbTab[] = {
	{ vetisdocstCREATED, "CREATED" },
	{ vetisdocstCONFIRMED, "CONFIRMED" },
	{ vetisdocstWITHDRAWN, "WITHDRAWN" },
	{ vetisdocstUTILIZED, "UTILIZED" },
	{ vetisdocstFINALIZED, "FINALIZED" }
};


int SLAPI PPVetisInterface::ParseNamedGenericVersioningEntity(xmlNode * pParentNode, VetisNamedGenericVersioningEntity & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "uuid", temp_buf))
			rResult.Uuid.FromStr(temp_buf);
		else if(SXml::GetContentByName(p_a, "guid", temp_buf))
			rResult.Guid.FromStr(temp_buf);
		else if(SXml::GetContentByName(p_a, "name", temp_buf))
			rResult.Name = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseEnterprise(xmlNode * pParentNode, VetisEnterprise & rResult)
{
	int    ok = 1;
	//SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	/*for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "uuid", temp_buf)) {
		}
		else if(SXml::GetContentByName(p_a, "guid", temp_buf)) {
		}
	}*/
	return ok;
}

int SLAPI PPVetisInterface::ParseBusinessEntity(xmlNode * pParentNode, VetisBusinessEntity & rResult)
{
	int    ok = 1;
	//SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	/*for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "uuid", temp_buf)) {
		}
		else if(SXml::GetContentByName(p_a, "guid", temp_buf)) {
		}
	}*/
	return ok;
}

int SLAPI PPVetisInterface::ParseBusinessMember(xmlNode * pParentNode, VetisBusinessMember & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "businessEntity")) {
			ParseBusinessEntity(p_a, rResult.BusinessEntity);
		}
		else if(SXml::IsName(p_a, "enterprise")) {
			ParseEnterprise(p_a, rResult.Enterprise);
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseProduct(xmlNode * pParentNode, VetisProduct & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "productType", temp_buf))
			rResult.ProductType = temp_buf.ToLong();
		else if(SXml::IsName(p_a, "code")) {
			rResult.Code = temp_buf;
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseSubProduct(xmlNode * pParentNode, VetisSubProduct & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "code")) {
			rResult.Code = temp_buf;
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseProductItem(xmlNode * pParentNode, VetisProductItem & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "name", temp_buf)) {
			rResult.Name = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseComplexDate(xmlNode * pParentNode, SUniTime & rResult)
{
	rResult.Z();
	int    ok = 1;
	int    year = 0;
	int    mon = 0;
	int    day = 0;
	int    hour = -1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "year", temp_buf))
			year = temp_buf.ToLong();
		else if(SXml::GetContentByName(p_a, "month", temp_buf))
			mon = temp_buf.ToLong();
		else if(SXml::GetContentByName(p_a, "day", temp_buf))
			day = temp_buf.ToLong();
		else if(SXml::GetContentByName(p_a, "hour", temp_buf))
			hour = temp_buf.ToLong();
	}
	if(year > 0) {
		if(mon >= 1 && mon <= 12) {
			if(day >= 1 && mon <= 31) {
				if(hour >= 0 && hour <= 24) {
					LDATETIME dtm;
					dtm.d.encode(day, mon, year);
					dtm.t.encode(hour, 0, 0, 0);
					rResult.Set(dtm, SUniTime::indHr);
				}
				else {
					LDATE dt;
					dt.encode(day, mon, year);
					rResult.Set(dt);
				}
			}
			else
				rResult.SetMonth(year, mon);
		}
		else
			rResult.SetYear(year);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseGoodsDate(xmlNode * pParentNode, VetisGoodsDate & rResult)
{
	int    ok = 1;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "firstDate"))
			ParseComplexDate(p_a, rResult.FirstDate);
		else if(SXml::IsName(p_a, "secondDate"))
			ParseComplexDate(p_a, rResult.SecondDate);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseBatch(xmlNode * pParentNode, VetisBatch & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "productType", temp_buf))
			rResult.ProductType = temp_buf.ToLong();
		else if(SXml::IsName(p_a, "product"))
			ParseProduct(p_a, rResult.Product);
		else if(SXml::IsName(p_a, "subProduct"))
			ParseSubProduct(p_a, rResult.SubProduct);
		else if(SXml::IsName(p_a, "productItem"))
			ParseProductItem(p_a, rResult.ProductItem);
		else if(SXml::GetContentByName(p_a, "volume", temp_buf))
			rResult.Volume = temp_buf.ToReal();
		else if(SXml::IsName(p_a, "dateOfProduction"))
			ParseGoodsDate(p_a, rResult.DateOfProduction);
		else if(SXml::IsName(p_a, "expiryDate"))
			ParseGoodsDate(p_a, rResult.ExpiryDate);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseCertifiedConsignment(xmlNode * pParentNode, VetisCertifiedConsignment & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "consignor")) {
			ParseBusinessMember(p_a, rResult.Consignor);
		}
		else if(SXml::IsName(p_a, "consignee")) {
			ParseBusinessMember(p_a, rResult.Consignee);
		}
		else if(SXml::IsName(p_a, "broker")) {
			ParseBusinessEntity(p_a, rResult.Broker);
		}
		else if(SXml::IsName(p_a, "batch")) {
			ParseBatch(p_a, rResult.Batch);
		}
		else if(SXml::IsName(p_a, "transportInfo")) {
		}
		else if(SXml::IsName(p_a, "transportStorageType")) {
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseVetDocument(xmlNode * pParentNode, VetisVetDocument & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "uuid", temp_buf))
			rResult.Uuid.FromStr(temp_buf);
		else if(SXml::GetContentByName(p_a, "issueDate", temp_buf))
			rResult.IssueDate = strtodate_(temp_buf, DATF_ISO8601);
		else if(SXml::GetContentByName(p_a, "form", temp_buf))
			rResult.VetDForm = SIntToSymbTab_GetId(VetisVetDocFormat_SymbTab, SIZEOFARRAY(VetisVetDocFormat_SymbTab), temp_buf);
		else if(SXml::GetContentByName(p_a, "type", temp_buf))
			rResult.VetDType = SIntToSymbTab_GetId(VetisVetDocType_SymbTab, SIZEOFARRAY(VetisVetDocType_SymbTab), temp_buf);
		else if(SXml::GetContentByName(p_a, "status", temp_buf))
			rResult.VetDStatus = SIntToSymbTab_GetId(VetisVetDocType_SymbTab, SIZEOFARRAY(VetisVetDocType_SymbTab), temp_buf);
		else if(SXml::IsName(p_a, "purpose")) {
		}
		else if(SXml::IsName(p_a, "confirmedBy")) {
		}
	}
	ParseCertifiedConsignment(pParentNode, rResult.CertifiedConsignment);
	return ok;
}

int SLAPI PPVetisInterface::ParseApplicationBlock(xmlNode * pParentNode, VetisApplicationBlock & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "applicationId", temp_buf))
			rResult.ApplicationId.FromStr(temp_buf);
		else if(ParseApplicationStatus(p_a, rResult.ApplicationStatus) > 0) {
		}
		else if(SXml::GetContentByName(p_a, "serviceId", temp_buf))
			rResult.ServiceId = temp_buf;
		else if(SXml::GetContentByName(p_a, "issuerId", temp_buf))
			rResult.IssuerId.FromStr(temp_buf);
		else if(SXml::GetContentByName(p_a, "issueDate", temp_buf))
			strtodatetime(temp_buf, &rResult.IssueDate, DATF_ISO8601, TIMF_HMS);
		else if(SXml::GetContentByName(p_a, "rcvDate", temp_buf))
			strtodatetime(temp_buf, &rResult.RcvDate, DATF_ISO8601, TIMF_HMS);
		else if(SXml::GetContentByName(p_a, "prdcRsltDate", temp_buf))
			strtodatetime(temp_buf, &rResult.PrdcRsltDate, DATF_ISO8601, TIMF_HMS);
		else if(SXml::IsName(p_a, "result")) {
			for(xmlNode * p_r = p_a->children; p_r; p_r = p_r->next) {
				if(SXml::IsName(p_r, "getVetDocumentListResponse")) {
					for(xmlNode * p_l = p_r->children; p_l; p_l = p_l->next) {
						if(SXml::IsName(p_l, "vetDocumentList")) {
							long   list_count = 0;
							long   list_offs = 0;
							long   total_count = 0;
							if(SXml::GetAttrib(p_l, "count", temp_buf) > 0)
								list_count = temp_buf.ToLong();
							else if(SXml::GetAttrib(p_l, "offset", temp_buf) > 0)
								list_offs = temp_buf.ToLong();
							else if(SXml::GetAttrib(p_l, "total", temp_buf) > 0)
								total_count = temp_buf.ToLong();
							for(xmlNode * p_i = p_l->children; p_i; p_i = p_i->next) {
								if(SXml::IsName(p_l, "vetDocument")) {

								}
							}
						}
					}
				}
				else {
				}
			}
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseReply(const SString & rReply, VetisApplicationBlock & rResult)
{
	int    ok = 1;
	xmlParserCtxt * p_ctx = 0;
	xmlDoc * p_doc = 0;
	rResult.Clear();
	SString temp_buf;
	xmlNode * p_root = 0;
	THROW(p_ctx = xmlNewParserCtxt());
	THROW_LXML(p_doc = xmlCtxtReadMemory(p_ctx, rReply, rReply.Len(), 0, 0, XML_PARSE_NOENT), p_ctx);
	THROW(p_root = xmlDocGetRootElement(p_doc));
	if(SXml::IsName(p_root, "Envelope")) {
		for(xmlNode * p_env = p_root->children; p_env; p_env = p_env->next) {
			if(SXml::IsName(p_env, "Body")) {
				for(xmlNode * p_b = p_env->children; p_b; p_b = p_b->next) {
					if(SXml::IsName(p_b, "receiveApplicationResultResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "application")) {
								THROW(ParseApplicationBlock(p_i, rResult));
							}
						}
					}
					else if(SXml::IsName(p_b, "submitApplicationResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "application")) {
								THROW(ParseApplicationBlock(p_i, rResult));
							}
						}
					}
					else if(SXml::IsName(p_b, "Fault")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::GetContentByName(p_i, "faultcode", temp_buf)) {
							}
							else if(SXml::GetContentByName(p_i, "faultstring", temp_buf)) {
							}
							else if(SXml::GetContentByName(p_i, "faultactor", temp_buf)) {
							}
							else if(SXml::GetContentByName(p_i, "detail", temp_buf)) {
								static const SIntToSymbTabEntry fault_tab[] = {
									{ VetisFault::tIncorrectRequest, "incorrectRequestFault" },
									{ VetisFault::tAccessDenied, "accessDeniedFault" },
									{ VetisFault::tEntityNotFound, "entityNotFoundFault" },
									{ VetisFault::tInternalService, "internalServiceFault" },
									{ VetisFault::tUnknownServiceId, "unknownServiceIdFault" },
									{ VetisFault::tUnsupportedApplicationDataType, "unsupportedApplicationDataTypeFault" },
								};
								for(xmlNode * p_d = p_i->children; p_d; p_d = p_d->next) {
									for(uint f = 0; f < SIZEOFARRAY(fault_tab); f++) {
										if(SXml::GetContentByName(p_d, fault_tab[f].P_Symb, temp_buf)) {
											VetisFault * p_fault = rResult.FaultList.CreateNewItem();
											THROW_SL(p_fault);
											p_fault->Type = fault_tab[f].Id;
											THROW(ParseFault(p_d, *p_fault));
											break;
										}
									}
								}
							}
						}
					}
					else {
					}
				}
			}
		}
	}
	xmlFreeDoc(p_doc);
	xmlFreeParserCtxt(p_ctx);
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::SubmitRequest(VetisApplicationBlock & rAppBlk, VetisApplicationBlock & rResult)
{
	int    ok = -1;
	SString temp_buf;
	THROW(State & stInited);
	THROW(rAppBlk.P_AppParam);
	{
		VetisSubmitRequestBlock srb;
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), "http://schemas.xmlsoap.org/soap/envelope/");
			n_env.PutAttrib(SXml::nst("xmlns", "ws"), "http://api.vetrf.ru/schema/cdm/application/ws-definitions");
			n_env.PutAttrib(SXml::nst("xmlns", "app"), "http://api.vetrf.ru/schema/cdm/application");
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst("ws", "submitApplicationRequest"));
					P.GetExtStrData(extssApiKey, temp_buf);
					n_f.PutInner(SXml::nst("ws", "apiKey"), temp_buf);
					{
						SXml::WNode n_app(srb, SXml::nst("app", "application"));
						n_app.PutInner(SXml::nst("app", "applicationId"), rAppBlk.ApplicationId.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf));
						n_app.PutInner(SXml::nst("app", "serviceId"), "mercury-g2b.service");
						n_app.PutInner(SXml::nst("app", "issuerId"), rAppBlk.IssuerId.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf));
						n_app.PutInner(SXml::nst("app", "issueDate"), temp_buf.Z().Cat(rAppBlk.IssueDate, DATF_ISO8601|DATF_CENTURY, /*TIMF_TIMEZONE*/0));
						SXml::WNode n_data(srb, SXml::nst("app", "data"));
						switch(rAppBlk.P_AppParam->Sign) {
							case VetisApplicationData::signGetVetDocumentList:
								{ 
									VetisGetVetDocumentListRequest * p_req = (VetisGetVetDocumentListRequest *)rAppBlk.P_AppParam;
									SXml::WNode n_req(srb, "getVetDocumentListRequest");

									//n_req.PutAttrib(SXml::nst("xmlns", "co"),  "http://api.vetrf.ru/schema/cdm/argus/common");
									//n_req.PutAttrib(SXml::nst("xmlns", "ent"), "http://api.vetrf.ru/schema/cdm/cerberus/enterprise");
									//n_req.PutAttrib(SXml::nst("xmlns", "vd"), "http://api.vetrf.ru/schema/cdm/mercury/vet-document");
									//n_req.PutAttrib(SXml::nst("xmlns", "bs"), "http://api.vetrf.ru/schema/cdm/base");
									n_req.PutAttrib(SXml::nst("xmlns", "sch"), "http://www.w3.org/2001/XMLSchema");
									n_req.PutAttrib(SXml::nst("xmlns", "vd"), "http://api.vetrf.ru/schema/cdm/mercury/vet-document");
									n_req.PutAttrib(SXml::nst("xmlns", "sh"), "http://api.vetrf.ru/schema/cdm/argus/shipment");
									n_req.PutAttrib(SXml::nst("xmlns", "ws"), "http://api.vetrf.ru/schema/cdm/application/ws-definitions");
									n_req.PutAttrib(SXml::nst("xmlns", "app"), "http://api.vetrf.ru/schema/cdm/application");
									n_req.PutAttrib(SXml::nst("xmlns", "co"), "http://api.vetrf.ru/schema/cdm/argus/common");
									n_req.PutAttrib(SXml::nst("xmlns", "ent"), "http://api.vetrf.ru/schema/cdm/cerberus/enterprise");
									n_req.PutAttrib(SXml::nst("xmlns", "pr"), "http://api.vetrf.ru/schema/cdm/argus/production");
									n_req.PutAttrib(SXml::nst("xmlns", "ik"), "http://api.vetrf.ru/schema/cdm/ikar"); 
									n_req.PutAttrib(SXml::nst("xmlns", "bs"), "http://api.vetrf.ru/schema/cdm/base");
									n_req.PutAttrib("xmlns", "http://api.vetrf.ru/schema/cdm/mercury/applications");

									n_req.PutInner("localTransactionId", temp_buf.Z().Cat(rAppBlk.LocalTransactionId));
									{
										SXml::WNode n_n2(srb, "initiator");
										n_n2.PutInner(SXml::nst("co", "login"), rAppBlk.User);
									}
									if(SIntToSymbTab_GetSymb(VetisVetDocType_SymbTab, SIZEOFARRAY(VetisVetDocType_SymbTab), p_req->DocType, temp_buf))
										n_req.PutInnerSkipEmpty(SXml::nst("vd", "vetDocumentType"), temp_buf);
									if(SIntToSymbTab_GetSymb(VetisVetDocStatus_SymbTab, SIZEOFARRAY(VetisVetDocStatus_SymbTab), p_req->DocStatus, temp_buf))
										n_req.PutInnerSkipEmpty(SXml::nst("vd", "vetDocumentStatus"), temp_buf);
									{
										SXml::WNode n_n2(srb, SXml::nst("bs", "listOptions"));
										n_n2.PutInner(SXml::nst("bs", "count"), temp_buf.Z().Cat(NZOR(p_req->ListOptions.Count, 100)));
										n_n2.PutInner(SXml::nst("bs", "offset"), temp_buf.Z().Cat(p_req->ListOptions.Offset));
									}
									n_req.PutInner(SXml::nst("ent", "enterpriseGuid"), temp_buf.Z().Cat(rAppBlk.EnterpriseId, S_GUID::fmtIDL|S_GUID::fmtLower));
								}
								break;
							default:
								CALLEXCEPT();
								break;
						}
					}
				}
			}
		}
		xmlTextWriterFlush(srb);
		rAppBlk.AppData.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
		THROW(SendSOAP(0, "submitApplicationRequest", rAppBlk.AppData, temp_buf));
		THROW(ParseReply(temp_buf, rResult));
	}
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::ReceiveResult(const S_GUID & rAppId, VetisApplicationBlock & rResult)
{
	int    ok = -1;
	SString temp_buf;
	SString reply_buf;
	THROW(State & stInited);
	{
		VetisSubmitRequestBlock srb;
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), "http://schemas.xmlsoap.org/soap/envelope/");
			n_env.PutAttrib(SXml::nst("xmlns", "ws"), "http://api.vetrf.ru/schema/cdm/application/ws-definitions");
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst("ws", "receiveApplicationResultRequest"));
					P.GetExtStrData(extssApiKey, temp_buf);
					n_f.PutInner(SXml::nst("ws", "apiKey"), temp_buf);
					n_f.PutInner(SXml::nst("ws", "issuerId"), P.IssuerUUID.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf));
					n_f.PutInner(SXml::nst("ws", "applicationId"), rAppId.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf));
				}
			}
		}
		xmlTextWriterFlush(srb);
		reply_buf.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
		THROW(SendSOAP(0, "receiveApplicationResult", reply_buf, temp_buf));
		THROW(ParseReply(temp_buf, rResult));
	}
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetProductQuery(int queryType, const char * pQueryParam, VetisProductItem & rResult)
{
	int    ok = -1;
	SString temp_buf;
	SString reply_buf;
	THROW(State & stInited);
	THROW(!isempty(pQueryParam));
	THROW(oneof6(queryType, qtProductItemByGuid, qtProductItemByUuid, qtProductByGuid, qtProductByUuid, qtSubProductByGuid, qtSubProductByUuid));
	{
		const char * p_soap_req = 0;
		const char * p_soap_action = 0;
		SString param_tag;
		VetisSubmitRequestBlock srb;
		switch(queryType) {
			case qtProductItemByGuid: 
				p_soap_req = "getProductItemByGuidRequest";
				p_soap_action = "GetProductItemByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtProductItemByUuid:
				p_soap_req = "getProductItemByUuidRequest";
				p_soap_action = "GetProductItemByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
			case qtProductByGuid:
				p_soap_req = "getProductByGuidRequest";
				p_soap_action = "GetProductByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtProductByUuid:
				p_soap_req = "getProductByUuidRequest";
				p_soap_action = "GetProductByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
			case qtSubProductByGuid:
				p_soap_req = "getSubProductByGuidRequest";
				p_soap_action = "GetSubProductByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtSubProductByUuid:
				p_soap_req = "getSubProductByUuidRequest";
				p_soap_action = "GetSubProductByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
		}
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), "http://schemas.xmlsoap.org/soap/envelope/");
			n_env.PutAttrib(SXml::nst("xmlns", "v2"), "http://api.vetrf.ru/schema/cdm/registry/ws-definitions/v2");
			n_env.PutAttrib(SXml::nst("xmlns", "base"), "http://api.vetrf.ru/schema/cdm/base");
			n_env.PutAttrib(SXml::nst("xmlns", "v21"), "http://api.vetrf.ru/schema/cdm/dictionary/v2");
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst("v2", p_soap_req));
					n_f.PutInner(param_tag, pQueryParam);
				}
			}
		}
		xmlTextWriterFlush(srb);
		reply_buf.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
		THROW(SendSOAP("https://api.vetrf.ru/platform/services/2.0/ProductService", p_soap_action, reply_buf, temp_buf));
		//THROW(ParseReply(temp_buf, rResult));
	}
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetProductItemList(uint offs, uint count, TSCollection <VetisProductItem> & rResult)
{
	int    ok = -1;
	SString temp_buf;
	SString reply_buf;
	THROW(State & stInited);
	{
		VetisSubmitRequestBlock srb;
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), "http://schemas.xmlsoap.org/soap/envelope/");
			n_env.PutAttrib(SXml::nst("xmlns", "v2"), "http://api.vetrf.ru/schema/cdm/registry/ws-definitions/v2");
			n_env.PutAttrib(SXml::nst("xmlns", "base"), "http://api.vetrf.ru/schema/cdm/base");
			n_env.PutAttrib(SXml::nst("xmlns", "v21"), "http://api.vetrf.ru/schema/cdm/dictionary/v2");
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst("v2", "getProductItemListRequest"));
					{
						SXml::WNode n_lo(srb, SXml::nst("base", "listOptions"));
						n_lo.PutInner(SXml::nst("base", "count"), temp_buf.Z().Cat(count));
						n_lo.PutInner(SXml::nst("base", "offset"), temp_buf.Z().Cat(offs));
					}
				}
			}
		}
		xmlTextWriterFlush(srb);
		reply_buf.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
		THROW(SendSOAP("https://api.vetrf.ru/platform/services/2.0/ProductService", "GetProductItemList", reply_buf, temp_buf));
		//THROW(ParseReply(temp_buf, rResult));
	}
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetRussianEnterpriseList(uint offs, uint count, TSCollection <VetisEnterprise> & rResult)
{
	int    ok = -1;
	/*
	SString user, password, api_key;
	TSCollection <VetisEnterprise> * p_result = 0; 
	PPSoapClientSession sess;
	VETIS_GETRUSSIANENTERPRISELIST_PROC func = 0;
	THROW(State & stInited);
	THROW(P_Lib);
	THROW_SL(func = (VETIS_GETRUSSIANENTERPRISELIST_PROC)P_Lib->GetProcAddr("Vetis_GetRussianEnterpriseList"));
	P.GetExtStrData(extssUser, user);
	P.GetExtStrData(extssPassword, password);
	sess.Setup(0, user, password);
	{
		VetisListOptionsRequest lstopt(VetisApplicationData::signGetRussianEnterpriseList);
		lstopt.ListOptions.Count = count;
		lstopt.ListOptions.Offset = offs;
		p_result = func(sess, 0, 0);
		THROW_PP_S(PreprocessResult(p_result, sess), PPERR_UHTTSVCFAULT, LastMsg);
		TSCollection_Copy(rResult, *p_result);
		DestroyResult((void **)&p_result);
		ok = 1;
	}
    CATCHZOK
	*/
	return ok;
}

int SLAPI PPVetisInterface::GetAppliedUserAuthorityList()
{
	int    ok = 1;
	/*
	VetisApplicationBlock submit_result;
	VetisApplicationBlock receive_result;
	THROW(SubmitRequest(VetisApplicationData::signGetAppliedUserAuthorityListRequest, 0, submit_result));
	if(submit_result.ApplicationStatus == VetisApplicationBlock::appstAccepted) {
		SDelay(1000);
		THROW(ReceiveResult(submit_result.ApplicationId, receive_result));
	}
	*/
	ok = -1;
	//CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetVetDocumentList(uint startOffset, uint count)
{
	int    ok = 1;
	SString temp_buf;
	VetisGetVetDocumentListRequest app_data;
	VetisApplicationBlock submit_result(0);
	VetisApplicationBlock receive_result(0);
	VetisApplicationBlock blk(&app_data);
	P.GetExtStrData(extssQInitiator, temp_buf);
	blk.User = temp_buf;
	blk.ServiceId = "mercury-g2b.service"; // "mercury-g2b.service:2.0";
	blk.IssuerId = P.IssuerUUID;
	blk.EnterpriseId = P.EntUUID;
	blk.IssueDate = getcurdatetime_();
	blk.ApplicationId.Generate();
	blk.LocalTransactionId = ++LastLocalTransactionId;
	app_data.ListOptions.Count = count;
	app_data.ListOptions.Offset = startOffset;
	THROW(SubmitRequest(blk, submit_result));
	if(submit_result.ApplicationStatus == VetisApplicationBlock::appstAccepted) {
		SDelay(1000);
		THROW(ReceiveResult(submit_result.ApplicationId, receive_result));
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetStockEntryList(uint startOffset, uint count)
{
	int    ok = 1;
	SString temp_buf;
	VetisApplicationData app_data(VetisApplicationData::signGetStockEntryList);
	VetisApplicationBlock submit_result(0);
	VetisApplicationBlock receive_result(0);
	VetisApplicationBlock blk(&app_data);
	P.GetExtStrData(extssQInitiator, temp_buf);
	blk.User = temp_buf;
	//blk.Func = appFuncId; //VetisApplicationBlock::detGetStockEntryListReq;
	blk.ServiceId = "mercury-g2b.service"; // "mercury-g2b.service:2.0";
	blk.IssuerId = P.IssuerUUID;
	blk.EnterpriseId = P.EntUUID;
	blk.IssueDate = getcurdatetime_();
	blk.ApplicationId.Generate();
	blk.LocalTransactionId = ++LastLocalTransactionId;
	{
		//...
	}
	THROW(SubmitRequest(blk, submit_result));
	if(submit_result.ApplicationStatus == VetisApplicationBlock::appstAccepted) {
		SDelay(1000);
		THROW(ReceiveResult(submit_result.ApplicationId, receive_result));
	}
	CATCHZOK
	return ok;
}

static void Debug_OutputProductItem(const VetisProductItem & rItem, SString & rBuf)
{
	rBuf.Z();
	rBuf.Cat(rItem.Uuid).Tab().Cat(rItem.Guid);
	if(rItem.GlobalID.NotEmpty())
		rBuf.Tab().Cat(rItem.GlobalID);
	if(rItem.Code.NotEmpty())
		rBuf.Tab().Cat(rItem.Code);
	rBuf.Tab().Cat(rItem.Name);
	rBuf.Tab().Cat(rItem.ProductType);
	if(rItem.Gost.NotEmpty())
		rBuf.Tab().Cat(rItem.Gost);
}

static void Debug_OutputEntItem(const VetisEnterprise & rItem, SString & rBuf)
{
	SString temp_buf;
	rBuf.Z();
	rBuf.Cat(rItem.Uuid).Tab().Cat(rItem.Guid);
	rBuf.Tab().Cat(rItem.Name).Tab().Cat(rItem.EnglishName);
	rBuf.Tab().Cat(rItem.Address.AddressView);
	rBuf.Tab();
	for(uint ssp = 0; rItem.NumberList.get(&ssp, temp_buf);) {
		rBuf.Cat(temp_buf).Space();
	}
}

int SLAPI TestVetis()
{
	int    ok = 1;
	SString temp_buf;
	TSCollection <VetisEnterprise> ent_list;
	TSCollection <VetisProductItem> productitem_list;
	PPVetisInterface::Param param;
	THROW(PPVetisInterface::SetupParam(param));
	{
		PPVetisInterface ifc;
		THROW(ifc.Init(param));
		//THROW(ifc.GetStockEntryList());
		//THROW(ifc.GetAppliedUserAuthorityList());
		{
			ifc.GetVetDocumentList(0, 100);
			//ifc.GetStockEntryList();
		}
		{
			//ifc.GetAppliedUserAuthorityList();
		}
		/*{
			PPGetFilePath(PPPATH_LOG, "vetis_ent_list.log", temp_buf);
			SFile f_out(temp_buf, SFile::mWrite);
			uint req_count = 50;
			for(uint req_offs = 0; ifc.GetRussianEnterpriseList(req_offs, req_count, ent_list);) {
				for(uint i = 0; i < ent_list.getCount(); i++) {
					const VetisEnterprise * p_item = ent_list.at(i);
					if(p_item) {
						Debug_OutputEntItem(*p_item, temp_buf);
						f_out.WriteLine(temp_buf.CR());
					}
				}
				if(ent_list.getCount() < req_count)
					break;
				else
					req_offs += ent_list.getCount();
			}
		}*/
		{
			PPGetFilePath(PPPATH_LOG, "vetis_product_list.log", temp_buf);
			SFile f_out(temp_buf, SFile::mWrite);
			uint req_count = 50;
			for(uint req_offs = 0; ifc.GetProductItemList(req_offs, req_count, productitem_list);) {
				for(uint i = 0; i < productitem_list.getCount(); i++) {
					const VetisProductItem * p_item = productitem_list.at(i);
					if(p_item) {
						Debug_OutputProductItem(*p_item, temp_buf);
						f_out.WriteLine(temp_buf.CR());
					}
				}
				if(productitem_list.getCount() < req_count)
					break;
				else
					req_offs += productitem_list.getCount();
			}
		}
	}
	CATCHZOK
	return ok;
}



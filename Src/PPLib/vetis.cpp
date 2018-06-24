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
	int    IsEmpty() const
	{
		return (Item.Empty() && Code.Empty() && Qualifier.Empty());
	}
	SString Item;
	SString Code;
	SString Qualifier;
};

struct VetisFault { 
	VetisFault() : Type(tUndef)
	{
	}
	int    IsEmpty() const
	{
		return (Message.Empty() && ErrList.getCount() == 0);
	}
	VetisFault & FASTCALL operator = (const VetisFault & rS)
	{
		Type = rS.Type;
		Message = rS.Message;
		TSCollection_Copy(ErrList, rS.ErrList);
		return *this;
	}
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
	VetisGenericEntity & Z()
	{
		Uuid.Z();
		return *this;
	}
	S_GUID Uuid;
};

struct VetisEnterpriseActivity : public VetisGenericEntity {
	VetisEnterpriseActivity & Z()
	{
		VetisGenericEntity::Z();
		Name.Z();
		return *this;
	}
	SString Name;
};

struct VetisGenericVersioningEntity : public VetisGenericEntity {
	VetisGenericVersioningEntity() : VetisGenericEntity(), Flags(0), Status(0), CreateDate(ZERODATETIME), UpdateDate(ZERODATETIME)
	{
	}
	VetisGenericVersioningEntity & Z()
	{
		VetisGenericEntity::Z();
		Guid.Z();
		Flags = 0;
		Status = 0;
		CreateDate.Z();
		UpdateDate.Z();
		Previous.Z();
		Next.Z();
		return *this;
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
	VetisNamedGenericVersioningEntity & Z()
	{
		VetisGenericVersioningEntity::Z();
		Name.Z();
		return *this;
	}
	SString Name;
};

struct VetisCountry : public VetisNamedGenericVersioningEntity {
	SString FullName;
	SString EnglishName;
	SString Code;
	SString Code3;
};

struct VetisFederalDistrict {
	VetisFederalDistrict & Z()
	{
		FullName.Z();
		ShortName.Z();
		Abbreviation.Z();
		return *this;
	}
	SString FullName;
	SString ShortName;
	SString Abbreviation;
};

struct VetisAddressObjectView : public VetisNamedGenericVersioningEntity {
	VetisAddressObjectView() : Flags(0)
	{
	}
	VetisAddressObjectView & Z()
	{
		VetisNamedGenericVersioningEntity::Z();
		EnglishName.Z();
		View.Z();
		RegionCode.Z();
		Type.Z();
		CountryGUID.Z();
		Flags = 0;
		return *this;
	}
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
	VetisDistrict & Z()
	{
		VetisAddressObjectView::Z();
		RegionGUID.Z();
		return *this;
	}
	S_GUID RegionGUID;
};

struct VetisLocality : public VetisAddressObjectView {
	VetisLocality::VetisLocality() : VetisAddressObjectView()
	{
	}
	VetisLocality & Z()
	{
		VetisAddressObjectView::Z();
		RegionGUID.Z();
		DistrictGUID.Z();
		CityGUID.Z();
		return *this;
	}
	S_GUID RegionGUID;
	S_GUID DistrictGUID;
	S_GUID CityGUID;
};

struct VetisStreet : public VetisAddressObjectView {
	VetisStreet() : VetisAddressObjectView()
	{
	}
	VetisStreet & Z()
	{
		VetisAddressObjectView::Z();
		LocalityGUID.Z();
		return *this;
	}
	S_GUID LocalityGUID;
};

struct VetisAddress {
	VetisAddress & Z()
	{
		Country.Z();
		FederalDistrict.Z();
		Region.Z();
		District.Z();
		Locality.Z();
		SubLocality.Z();
		Street.Z();
		House.Z();
		Building.Z();
		Room.Z();
		PostIndex.Z();
		PostBox.Z();
		AdditionalInfo.Z();
		AddressView.Z();
		EnAddressView.Z();
		return *this;
	}
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
	VetisDocument() : VetisGenericEntity(), EntityID(0), IssueDate(ZERODATE), DocumentType(0)
	{
	}
	VetisDocument & FASTCALL operator = (const VetisDocument & rS)
	{
		VetisGenericEntity::operator = (rS);
		EntityID = rS.EntityID;
		Name = rS.Name;
		IssueSeries = rS.IssueSeries;
		IssueNumber = rS.IssueNumber;
		DocumentForm = rS.DocumentForm;
		IssueDate = rS.IssueDate;
		DocumentType = rS.DocumentType;
		Issuer = rS.Issuer;
		For_ = rS.For_;
		Qualifier = rS.Qualifier;
		return *this;
	}
	PPID   EntityID;   // ID документа в собственной базе данных
	SString Name;
	SString IssueSeries;
	SString IssueNumber;
	SString DocumentForm;
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
	VetisIncorporationForm & Z()
	{
		VetisGenericEntity::Z();
		Name.Z();
		ShortName.Z();
		Code.Z();
		return *this;
	}
	SString Name;
	SString ShortName;
	SString Code;
};

struct VetisBusinessEntity : public VetisNamedGenericVersioningEntity {
	VetisBusinessEntity() : VetisNamedGenericVersioningEntity(), EntityID(0), Type(0)
	{
	}
	VetisBusinessEntity & Z()
	{
		VetisNamedGenericVersioningEntity::Z();
		EntityID = 0;
		Type = 0;
		FullName.Z();
		Fio.Z();
		Passport.Z();
		Inn.Z();
		Kpp.Z();
		Ogrn.Z();
		IncForm.Z();
		JuridicalAddress.Z();
		ActivityLocationList.freeAll();
		return *this;
	}
	VetisBusinessEntity & FASTCALL operator = (const VetisBusinessEntity & rS)
	{
		VetisNamedGenericVersioningEntity::operator = (rS);
		EntityID = rS.EntityID;
		Type = rS.Type;
		FullName = rS.FullName;
		Fio = rS.Fio;
		Passport = rS.Passport;
		Inn = rS.Inn;
		Kpp = rS.Kpp;
		Ogrn = rS.Ogrn;
		IncForm = rS.IncForm;
		JuridicalAddress = rS.JuridicalAddress;
		TSCollection_Copy(ActivityLocationList, rS.ActivityLocationList);
		return *this;
	}
	PPID   EntityID;
	int    Type;     // BusinessEntityType
	SString FullName;
	SString Fio;
	SString Passport;
	SString Inn;
	SString Kpp;
	SString Ogrn;
	VetisIncorporationForm IncForm;
	VetisAddress JuridicalAddress;
	TSCollection <VetisBusinessEntity_activityLocation> ActivityLocationList;
};

struct VetisListOptions {
	VetisListOptions() : Count(0), Offset(0)
	{
	}
	void   Set(uint o, uint c)
	{
		Offset = o;
		Count = c;
	}
	uint   Count;
	uint   Offset;
};

enum VetisTransportStorageType { // @persistent (vetis - идентификация по символу, в БД Papyrus - по значению)
	vtstUNDEF        = 0, // Не определенный
	vtstFROZEN       = 1, // Замороженный
	vtstCHILLED      = 2, // Охлажденный
	vtstCOOLED       = 3, // Охлаждаемый
	vtstVENTILATED   = 4, // Вентилируемый
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
	VetisProductItem() : VetisNamedGenericVersioningEntity(), EntityID(0), NativeGoodsID(0), ProductType(vptUndef)
	{
	}
	VetisProductItem & FASTCALL operator = (const VetisProductItem & rS)
	{
		VetisNamedGenericVersioningEntity::operator = (rS);
		EntityID = rS.EntityID;
		NativeGoodsID = rS.NativeGoodsID;
		ProductType = rS.ProductType;
		GlobalID = rS.GlobalID;
		Code = rS.Code;
		Gost = rS.Gost;
		Product = rS.Product;
		SubProduct = rS.SubProduct;
		Producer = rS.Producer;
		TmOwner = rS.TmOwner;
		Packaging = rS.Packaging;
		TSCollection_Copy(Producing, rS.Producing);
		return *this;
	}
	enum {
		fCorrespondsToGost = 0x00010000,
		fIsPublic          = 0x00020000
	};
	PPID   EntityID;   // ID элемента в собственной базе данных (VetisEntityTbl)
	PPID   NativeGoodsID;
	int    ProductType; // ProductType vptXXX
	SString GlobalID;
	SString Code;
	SString Gost;
	VetisProduct Product;
	VetisSubProduct SubProduct;
	VetisBusinessEntity Producer;
	VetisBusinessEntity TmOwner;
	TSCollection <VetisEnterprise> Producing;
	VetisPackaging Packaging;
};

struct VetisGoodsDate {
	VetisGoodsDate()
	{
	}
	SUniTime FirstDate;
	SUniTime SecondDate;
	SString InformalDate;
};

struct VetisProducer : public VetisEnterprise {
	VetisProducer() : Role(0)
	{
	}
	enum {
		roleUNKNOWN  = 0,
		rolePRODUCER = 1
	};
	int    Role; // EnterpriseRole
};

struct VetisBatchOrigin {
	VetisBatchOrigin & FASTCALL operator = (const VetisBatchOrigin & rS)
	{
		ProductItem = rS.ProductItem;
		Country = rS.Country;
		TSCollection_Copy(Producer, rS.Producer);
		return *this;
	}
	VetisProductItem ProductItem;
	VetisCountry Country;
	TSCollection <VetisProducer> Producer;
};

struct VetisBatch {
	VetisBatch() : NativeGoodsID(0), ProductType(vptUndef), Volume(0.0), PackingAmount(0), Flags(0), P_Owner(0)
	{
	}
	~VetisBatch()
	{
		delete P_Owner;
	}
	VetisBatch & FASTCALL operator = (const VetisBatch & rS)
	{
		NativeGoodsID = rS.NativeGoodsID;
		ProductType = rS.ProductType;
		Product = rS.Product;
		SubProduct = rS.SubProduct;
		ProductItem = rS.ProductItem;
		Volume = rS.Volume;
		PackingAmount = rS.PackingAmount;
		Unit = rS.Unit;
		DateOfProduction = rS.DateOfProduction;
		ExpiryDate = rS.ExpiryDate;
		ProductMarkingList = rS.ProductMarkingList;
		Flags = rS.Flags;
		Origin = rS.Origin;
		//TSCollection_Copy(PackageList, rS.PackageList);
		TSCollection_Copy(PackingList, rS.PackingList);
		ZDELETE(P_Owner);
		if(rS.P_Owner) {
			P_Owner = new VetisBusinessEntity;
			*P_Owner = *rS.P_Owner;
		}
		return *this;
	}
	PPID   NativeGoodsID; // Ид товара, сопоставленного с товаром, обозначенным в этой структуре (далеко не всегда возможно сделать автоматом)
	int    ProductType; // ProductType vptXXX
	VetisProduct Product;
	VetisSubProduct SubProduct;
	VetisProductItem ProductItem;
	double Volume;
	long   PackingAmount;
	VetisUnit Unit;
	VetisGoodsDate DateOfProduction;
	VetisGoodsDate ExpiryDate;
	//StringSet BatchIdList;
	StringSet ProductMarkingList; // Список маркировочных кодов продукции (при разборе входящих пакетов
		// вносим в этот список только коды, удовлетворяющие стандартам EAN13, EAN8, UPCA, UPCE).
	enum {
		fPerishable    = 0x0001,
		fLowGradeCargo = 0x0002
	};
	long   Flags;
	VetisBatchOrigin Origin;
	//TSCollection <VetisPackage> PackageList;
	TSCollection <VetisNamedGenericVersioningEntity> PackingList;
	VetisBusinessEntity * P_Owner;
};

struct VetisBusinessMember {
	VetisBusinessMember & FASTCALL operator = (const VetisBusinessMember & rS)
	{
		BusinessEntity = rS.BusinessEntity;
		Enterprise = rS.Enterprise;
		GlobalID = rS.GlobalID;
		return *this;
	}
	VetisBusinessEntity BusinessEntity;
	VetisEnterprise Enterprise;
	SString GlobalID;
};

struct VetisCertifiedBatch : public VetisBatch {
	VetisBusinessMember Producer;
};

struct VetisTransportNumber {
	int    SLAPI IsEmpty() const
	{
		return (ContainerNumber.Empty() && WagonNumber.Empty() && VehicleNumber.Empty() && 
			TrailerNumber.Empty() && ShipName.Empty() && FlightNumber.Empty());
	}
	SString ContainerNumber;
	SString WagonNumber;
	SString VehicleNumber;
	SString TrailerNumber;
	SString ShipName;
	SString FlightNumber;
};

struct VetisTransportInfo {
	enum { // @persistent
		ttCar     = 1,
		ttRailWay = 2,
		ttAvia    = 3,
		ttShip    = 4
	};
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
	VetisCertifiedConsignment & operator = (const VetisCertifiedConsignment & rS)
	{
		Consignor = rS.Consignor;
		Consignee = rS.Consignee;
		Broker = rS.Broker;
		TransportInfo = rS.TransportInfo;
		TransportStorageType = rS.TransportStorageType;
		TSCollection_Copy(RoutePointList, rS.RoutePointList);
		Batch = rS.Batch;
		return *this;
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
	VetisVeterinaryAuthentication & FASTCALL operator = (const VetisVeterinaryAuthentication & rS)
	{
		Purpose = rS.Purpose;
		Flags = rS.Flags;
		CargoExpertized = rS.CargoExpertized;
		AnimalSpentPeriod = rS.AnimalSpentPeriod;
		LocationProsperity = rS.LocationProsperity;
		MonthsSpent = rS.MonthsSpent;
		SpecialMarks = rS.SpecialMarks;
		TSCollection_Copy(LaboratoryResearchList, rS.LaboratoryResearchList);
		TSCollection_Copy(ImmunizationList, rS.ImmunizationList);
		TSCollection_Copy(VeterinaryEventList, rS.VeterinaryEventList);
		TSCollection_Copy(R13nClauseList, rS.R13nClauseList);
		return *this;
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
		UnionVetDocument(0), P_CertifiedBatch(0), WayBillDate(ZERODATE)
	{
	}
	~VetisVetDocument()
	{
		delete P_CertifiedBatch;
	}
	VetisVetDocument & FASTCALL operator = (const VetisVetDocument & rS)
	{
		VetisDocument::operator = (rS);
		VetDForm = rS.VetDForm;
		VetDType = rS.VetDType;
		VetDStatus = rS.VetDStatus;
		Flags = rS.Flags;
		LastUpdateDate = rS.LastUpdateDate;
		WayBillType = rS.WayBillType;
		WayBillDate = rS.WayBillDate;
		WayBillNumber = rS.WayBillNumber;
		WayBillSeries = rS.WayBillSeries;
		UnionVetDocument = rS.UnionVetDocument;
		ZDELETE(P_CertifiedBatch);
		if(rS.P_CertifiedBatch) {
			P_CertifiedBatch = new VetisCertifiedBatch;
			*P_CertifiedBatch = *rS.P_CertifiedBatch;
		}
		CertifiedConsignment = rS.CertifiedConsignment;
		//TSCollection_Copy(ActivityLocationList, rS.ActivityLocationList);
		return *this;
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
		fFinalized       = 0x00010000,
		fCargoInspected  = 0x00020000,
		fCargoExpertized = 0x00040000,
		fFromMainOrg     = 0x00100000,
		fToMainOrg       = 0x00200000
	};
	long   Flags;
	LDATETIME LastUpdateDate;
	//
	enum {
		waybilltTTN         = 1,
		waybilltConsignment = 2,
		waybilltCMR         = 3,
		waybilltAvia        = 4
	};
	int    WayBillType;
	LDATE  WayBillDate;
	SString WayBillNumber;
	SString WayBillSeries;
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
		signGetVetDocumentChangesList,
		signGetProductItemList,
		signGetVetDocumentByUuid,
		signProcessIncomingConsignment
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

class VetisGetVetDocumentChangesListRequest : public VetisApplicationData {
public:
	VetisGetVetDocumentChangesListRequest() : VetisApplicationData(signGetVetDocumentChangesList), DocType(-1), DocStatus(-1)
	{
	}
	VetisListOptions ListOptions;
	int    DocType;   // VetisDocType
	int    DocStatus; // VetisDocStatus
	STimeChunk Period;
};

class VetisGetVetDocumentByUuidRequest : public VetisApplicationData {
public:
	VetisGetVetDocumentByUuidRequest() : VetisApplicationData(signGetVetDocumentByUuid)
	{
	}
	S_GUID Uuid;
};

class VetisProcessIncomingConsignmentRequest : public VetisApplicationData {
public:
	VetisProcessIncomingConsignmentRequest() : VetisApplicationData(signProcessIncomingConsignment)
	{
	}
	VetisVetDocument Doc;
};

struct VetisApplicationBlock {
	VetisApplicationBlock(uint vetisSvcVer, const VetisApplicationData * pAppParam);
	VetisApplicationBlock();
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
	struct  ReplyListValues {
		long   Count;
		long   Offset;
		long   Total;
	};
	uint   VetisSvcVer; // 1, 2
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
	ReplyListValues ListResult;
	SString AppData; // xml-block

	TSCollection <VetisErrorEntry> ErrList;
	TSCollection <VetisFault> FaultList;
	const VetisApplicationData * P_AppParam;

	TSCollection <VetisProductItem> ProductItemList; // Ответ на запрос одного или нескольких ProductItem
	TSCollection <VetisEnterprise> EntItemList;
	TSCollection <VetisBusinessEntity> BEntList;
	TSCollection <VetisVetDocument> VetDocList;
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

VetisApplicationBlock::VetisApplicationBlock(uint vetisSvcVer, const VetisApplicationData * pAppParam) : 
	VetisSvcVer(vetisSvcVer), ApplicationStatus(appstUndef), IssueDate(ZERODATETIME), RcvDate(ZERODATETIME), 
	PrdcRsltDate(ZERODATETIME), P_AppParam(pAppParam), LocalTransactionId(0)
{
	MEMSZERO(ListResult);
}

VetisApplicationBlock::VetisApplicationBlock() : 
	VetisSvcVer(1), ApplicationStatus(appstUndef), IssueDate(ZERODATETIME), RcvDate(ZERODATETIME), 
	PrdcRsltDate(ZERODATETIME), P_AppParam(0), LocalTransactionId(0)
{
	MEMSZERO(ListResult);
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
	ProductItemList.freeAll();
	EntItemList.freeAll();
	BEntList.freeAll();
	VetDocList.freeAll();
	MEMSZERO(ListResult);
	AppData.Z();
}

int FASTCALL VetisApplicationBlock::Copy(const VetisApplicationBlock & rS)
{
	int    ok = 1;
	VetisSvcVer = rS.VetisSvcVer;
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
	TSCollection_Copy(ProductItemList, rS.ProductItemList);
	TSCollection_Copy(EntItemList, rS.EntItemList);
	TSCollection_Copy(BEntList, rS.BEntList);
	TSCollection_Copy(VetDocList, rS.VetDocList);
	ListResult = rS.ListResult;
	AppData = rS.AppData;
	P_AppParam = rS.P_AppParam;
	return ok;
}

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

SLAPI VetisEntityCore::Entity::Entity() : ID(0), Kind(kUndef), Flags(0), Status(0), GuidRef(0), UuidRef(0)
{
}

SLAPI VetisEntityCore::Entity::Entity(int kind, const VetisProductItem & rS) : ID(0), Kind(kind), Status(rS.Status), Flags(rS.Flags)
{
	assert(oneof3(kind, kProductItem, kProduct, kSubProduct));
	if(kind == kProductItem) {
		Guid = rS.Guid;
		Uuid = rS.Uuid;
		Name = rS.Name;
	}
	else if(kind == kProduct) {
		Guid = rS.Product.Guid;
		Uuid = rS.Product.Uuid;
		Name = rS.Product.Name;
	}
	else if(kind == kSubProduct) {
		Guid = rS.SubProduct.Guid;
		Uuid = rS.SubProduct.Uuid;
		Name = rS.SubProduct.Name;
	}
}

SLAPI VetisEntityCore::Entity::Entity(int kind, const VetisNamedGenericVersioningEntity & rS) : ID(0), Kind(kind), Status(rS.Status), Flags(rS.Flags)
{
	Guid = rS.Guid;
	Uuid = rS.Uuid;
	Name = rS.Name;
}

SLAPI VetisEntityCore::Entity::Entity(const VetisVetDocument & rS) : ID(0), Kind(kVetDocument), Status(0), Flags(rS.Flags)
{
	Uuid = rS.Uuid;
}

void FASTCALL VetisEntityCore::Entity::Get(VetisNamedGenericVersioningEntity & rD) const
{
	rD.Guid = Guid;
	rD.Uuid = Uuid;
	rD.Name = Name;
}

VetisEntityCore::Entity & SLAPI VetisEntityCore::Entity::Z()
{
	ID = 0;
	Kind = 0;
	Guid.Z();
	Uuid.Z();
	Flags = 0;
	Status = 0;
	GuidRef = 0;
	UuidRef = 0;
	Name.Z();
	return *this;
}

SLAPI VetisEntityCore::VetisEntityCore()
{
}

//static 
int FASTCALL VetisEntityCore::ValidateEntityKind(int kind)
{
	return oneof9(kind, kProductItem, kProduct, kSubProduct, kEnterprise, kBusinessEntity, kUOM, kCountry, kRegion, kVetDocument);
}

int SLAPI VetisEntityCore::CollectUnresolvedEntityList(TSVector <UnresolvedEntity> & rList)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	SStringU temp_buf_u;
	VetisEntityTbl::Key0 k0;
	MEMSZERO(k0);
	BExtQuery q(&ET, 0);
	q.selectAll();
	for(q.initIteration(0, &k0, spFirst); q.nextIteration() > 0;) {
		VetisEntityTbl::Rec rec;
		ET.copyBufTo(&rec);
		if(oneof5(rec.Kind, kProductItem, kProduct, kSubProduct, kEnterprise, kBusinessEntity)) {
			if(rec.GuidRef || rec.UuidRef) {
				if(!rList.lsearch(&rec.ID, 0, CMPF_LONG)) {
					TextRefIdent tri(PPOBJ_VETISENTITY, rec.ID, PPTRPROP_NAME);
					if(p_ref->TrT.Search(tri, temp_buf_u) > 0 && temp_buf_u.Len())
						;
					else {
						UnresolvedEntity ure_entry;
						ure_entry.ID = rec.ID;
						ure_entry.Kind = rec.Kind;
						ure_entry.GuidRef = rec.GuidRef;
						ure_entry.UuidRef = rec.UuidRef;
						THROW_SL(rList.insert(&ure_entry));
					}
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI VetisEntityCore::GetEntityByGuid(const S_GUID & rGuid, Entity & rE)
{
	int     ok = -1;
	long    uuid_id = 0;
	if(UrT.SearchUuid(rGuid, 0, &uuid_id) > 0) {
		VetisEntityTbl::Key1 k1;
		MEMSZERO(k1);
		k1.GuidRef = uuid_id;
		if(ET.search(1, &k1, spGe) && ET.data.GuidRef == uuid_id) {
			EntityRecToEntity(ET.data, rE);
			ok = 1;
		}
	}
	return ok;
}

int SLAPI VetisEntityCore::GetEntityByUuid(const S_GUID & rUuid, Entity & rE)
{
	int     ok = -1;
	long    uuid_id = 0;
	if(UrT.SearchUuid(rUuid, 0, &uuid_id) > 0) {
		VetisEntityTbl::Key1 k2;
		MEMSZERO(k2);
		k2.UuidRef = uuid_id;
		if(ET.search(2, &k2, spGe) && ET.data.UuidRef == uuid_id) {
			EntityRecToEntity(ET.data, rE);
			ok = 1;
		}
	}
	return ok;
}

int SLAPI VetisEntityCore::EntityRecToEntity(const VetisEntityTbl::Rec & rRec, Entity & rE)
{
	int    ok = 1;
	rE.ID = rRec.ID;
	rE.Kind = rRec.Kind;
	rE.Flags = rRec.Flags;
	rE.Status = rRec.Status;
	rE.GuidRef = rRec.GuidRef;
	rE.UuidRef = rRec.UuidRef;
	UrT.Search(rRec.GuidRef, rE.Guid);
	UrT.Search(rRec.UuidRef, rE.Uuid);
	{
		SStringU temp_buf_u;
		TextRefIdent tri(PPOBJ_VETISENTITY, rRec.ID, PPTRPROP_NAME);
		PPRef->TrT.Search(tri, temp_buf_u);
		if(temp_buf_u.Len()) {
			temp_buf_u.CopyToUtf8(rE.Name, 0);
			rE.Name.Transf(CTRANSF_UTF8_TO_INNER);
		}
		else
			rE.Name.Z();
	}
	return ok;
}

int SLAPI VetisEntityCore::GetEntity(PPID id, Entity & rE)
{
	rE.Z();
	VetisEntityTbl::Rec rec;
	int    ok = SearchByID(&ET, PPOBJ_VETISENTITY, id, &rec);
	if(ok > 0) {
		EntityRecToEntity(rec, rE);
	}
	return ok;
}

int SLAPI VetisEntityCore::SetEntity(Entity & rE, TSVector <UnresolvedEntity> * pUreList, int use_ta)
{
	int    ok = 1;
	int    is_new_entity = 0;
	int    is_unresolved = 0;
	Reference * p_ref = PPRef;
	SString temp_buf;
	SString name_buf;
	SStringU temp_buf_u;
	VetisEntityTbl::Rec rec;
	PPID   entity_id = 0;
	assert(ValidateEntityKind(rE.Kind));
	THROW(ValidateEntityKind(rE.Kind));
	if(!rE.Guid && !rE.Uuid) {
		rE.ID = 0;
	}
	else {
		PPTransaction tra(use_ta);
		THROW(tra);
		MEMSZERO(rec);
		THROW(UrT.GetUuid(rE.Guid, &rec.GuidRef, 0, 0));
		THROW(UrT.GetUuid(rE.Uuid, &rec.UuidRef, 0, 0));
		rE.GuidRef = rec.GuidRef;
		rE.UuidRef = rec.UuidRef;
		rec.Flags = rE.Flags;
		rec.Kind = rE.Kind;
		rec.Status = rE.Status;
		{
			VetisEntityTbl::Key1 k1;
			k1.GuidRef = rec.GuidRef;
			k1.UuidRef = rec.UuidRef;
			if(ET.searchForUpdate(1, &k1, spEq)) {
				entity_id = ET.data.ID;
				assert(rE.ID == 0 || entity_id == rE.ID);
				rE.ID = entity_id;
				rec.ID = entity_id;
				if(memcmp(&ET.data, &rec, sizeof(rec)) != 0) {
					THROW_DB(ET.updateRecBuf(&rec)); // @sfu
				}
				else
					ok = -1;
			}
			else {
				THROW_DB(ET.insertRecBuf(&rec, 0, &entity_id));
				rE.ID = entity_id;
				is_new_entity = 1;
			}
		}
		name_buf = rE.Name;
		if(name_buf.NotEmptyS()) {
			name_buf.Trim(124);
			temp_buf_u.CopyFromUtf8(name_buf.ToLower().Transf(CTRANSF_INNER_TO_UTF8));
			TextRefIdent tri(PPOBJ_VETISENTITY, entity_id, PPTRPROP_NAME);
			int sr = p_ref->TrT.SetText(tri, temp_buf_u, 0);
			THROW(sr);
			if(sr > 0 && ok < 0)
				ok = 1;
		}
		else if(is_new_entity)
			is_unresolved = 1;
		THROW(tra.Commit());
		if(pUreList && rE.Kind != kVetDocument) {
			if(!is_unresolved) {
				TextRefIdent tri(PPOBJ_VETISENTITY, entity_id, PPTRPROP_NAME);
				if(p_ref->TrT.Search(tri, temp_buf_u) > 0 && temp_buf_u.Len())
					;
				else
					is_unresolved = 1;
			}
			if(is_unresolved && (is_new_entity || !pUreList->lsearch(&entity_id, 0, CMPF_LONG))) {
				UnresolvedEntity ure_entry;
				ure_entry.ID = entity_id;
				ure_entry.Kind = rE.Kind;
				ure_entry.GuidRef = rE.GuidRef;
				ure_entry.UuidRef = rE.UuidRef;
				THROW_SL(pUreList->insert(&ure_entry));
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI VetisEntityCore::SearchPerson(PPID id, VetisPersonTbl::Rec * pRec)
	{ return SearchByID(&BT, 0, id, pRec); }
int SLAPI VetisEntityCore::SearchDocument(PPID id, VetisDocumentTbl::Rec * pRec)
	{ return SearchByID(&DT, 0, id, pRec); }

int SLAPI VetisEntityCore::MatchDocument(PPID docEntityID, PPID billID, int rowN, int use_ta)
{
	int    ok = -1;
	Reference * p_ref = PPRef;
	VetisDocumentTbl::Rec rec;
	VetisDocumentTbl::Rec org_rec;
	VetisDocumentTbl::Key0 k0;
	VetisDocumentTbl::Key1 k1;
	TransferTbl::Rec trfr_rec;
	k0.EntityID = docEntityID;
	int16   row_n = 0;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(rowN <= 0) {
			row_n = -1;
			MEMSZERO(trfr_rec);
		}
		else {
			row_n = rowN;
			THROW(BillObj->trfr->SearchByBill(billID, 0, row_n, &trfr_rec) > 0);
			THROW(trfr_rec.Flags & PPTFR_RECEIPT && trfr_rec.LotID != 0);
		}
		if(DT.searchForUpdate(0, &k0, spEq)) {
			DT.copyBufTo(&org_rec);
			DT.copyBufTo(&rec);
			rec.LinkBillID = billID;
			rec.LinkBillRow = row_n;
			if(row_n > 0) {
				rec.LinkGoodsID = trfr_rec.GoodsID;
			}
			THROW_DB(DT.updateRecBuf(&rec));
			if(trfr_rec.LotID) {
				Entity entity;
				if(GetEntity(docEntityID, entity) > 0 && !!entity.Uuid) {
					ObjTagItem tag_item;
					if(tag_item.SetGuid(PPTAG_LOT_VETIS_UUID, &entity.Uuid)) {
						THROW(p_ref->Ot.PutTag(PPOBJ_LOT, trfr_rec.LotID, &tag_item, 0));
					}
				}
			}
			if(row_n < 0) {
				if(checkdate(rec.WayBillDate) && rec.WayBillNumber[0]) {
					PPIDArray additional_to_upd_list;
					VetisDocumentTbl::Key3 k3;
					MEMSZERO(k3);
					k3.WayBillDate = rec.WayBillDate;
					STRNSCPY(k3.WayBillNumber, rec.WayBillNumber);
					if(DT.search(3, &k3, spGe) && DT.data.WayBillDate == rec.WayBillDate && sstreq(rec.WayBillNumber, DT.data.WayBillNumber)) do {
						if(DT.data.EntityID != rec.EntityID && (DT.data.LinkBillID == 0 /*|| DT.data.LinkBillID == org_rec.LinkBillID*/)) {
							additional_to_upd_list.add(DT.data.EntityID);
						}
					} while(DT.search(3, &k3, spNext) && DT.data.WayBillDate == rec.WayBillDate && sstreq(rec.WayBillNumber, DT.data.WayBillNumber));
					for(uint i = 0; i < additional_to_upd_list.getCount(); i++) {
						const PPID additional_to_upd_id = additional_to_upd_list.get(i);
						THROW(MatchDocument(additional_to_upd_id, billID, rowN, 0));
					}
				}
			}
		}
		THROW(tra.Commit())
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI VetisEntityCore::MatchPersonInDocument(PPID docEntityID, int side /*0 - from, 1 - to*/, PPID personID, PPID dlvrLocID, int use_ta)
{
	assert(oneof2(side, 0, 1));
	int    ok = -1;
	Reference * p_ref = PPRef;
	VetisDocumentTbl::Rec rec;
	VetisDocumentTbl::Key0 k0;
	VetisPersonTbl::Key0 vp_key0;
	k0.EntityID = docEntityID;
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		if(DT.searchForUpdate(0, &k0, spEq)) {
			DT.copyBufTo(&rec);
			Entity entity;
			ObjTagItem tag_item;
			int    is_psn_guid_set = 0;
			if(side == 0) { // FROM
				const  PPID base_entity_id = rec.FromEnterpriseID;
				if(rec.LinkFromPsnID != personID || rec.LinkFromDlvrLocID != dlvrLocID) {
					rec.LinkFromPsnID = personID;
					rec.LinkFromDlvrLocID = dlvrLocID;
					THROW_DB(DT.updateRecBuf(&rec));
					ok = 1;
				}
				if(GetEntity(base_entity_id, entity) > 0 && !!entity.Guid) {
					if(dlvrLocID) {
						if(tag_item.SetGuid(PPTAG_LOC_VETIS_GUID, &entity.Guid)) {
							THROW(p_ref->Ot.PutTag(PPOBJ_LOCATION, dlvrLocID, &tag_item, 0));
							ok = 1;
						}
					}
					else if(personID) {
						if(tag_item.SetGuid(PPTAG_PERSON_VETISUUID, &entity.Guid)) {
							THROW(p_ref->Ot.PutTag(PPOBJ_PERSON, personID, &tag_item, 0));
							ok = 1;
							is_psn_guid_set = 1;
						}
					}
				}
				if(!is_psn_guid_set && GetEntity(rec.FromEntityID, entity) > 0 && !!entity.Guid) {
					if(personID && tag_item.SetGuid(PPTAG_PERSON_VETISUUID, &entity.Guid)) {
						THROW(p_ref->Ot.PutTag(PPOBJ_PERSON, personID, &tag_item, 0));
						ok = 1;
					}
				}
				{
					const int idx = 5;
					VetisDocumentTbl::Key5 key;
					if(base_entity_id) {
						MEMSZERO(key);
						key.FromEnterpriseID = base_entity_id;
						if(DT.searchForUpdate(idx, &key, spGe) && DT.data.FromEnterpriseID == base_entity_id) do {
							if(DT.data.LinkFromPsnID != personID || DT.data.LinkFromDlvrLocID != dlvrLocID) {
								DT.data.LinkFromPsnID = personID;
								DT.data.LinkFromDlvrLocID = dlvrLocID;
								THROW_DB(DT.updateRec());
								ok = 1;
							}
						} while(DT.searchForUpdate(idx, &key, spNext) && DT.data.FromEnterpriseID == base_entity_id);
						{
							MEMSZERO(vp_key0);
							vp_key0.EntityID = base_entity_id;
							if(BT.searchForUpdate(0, &vp_key0, spEq) && (BT.data.LinkPersonID != personID || BT.data.LinkDlvrLocID != dlvrLocID)) {
								BT.data.LinkPersonID = personID;
								BT.data.LinkDlvrLocID = dlvrLocID;
								THROW_DB(BT.updateRec());
								ok = 1;
							}
						}
					}
				}
			}
			else { // TO
				const  PPID base_entity_id = rec.ToEnterpriseID;
				if(rec.LinkToPsnID != personID || rec.LinkToDlvrLocID != dlvrLocID) {
					rec.LinkToPsnID = personID;
					rec.LinkToDlvrLocID = dlvrLocID;
					THROW_DB(DT.updateRecBuf(&rec));
					ok = 1;
				}
				if(GetEntity(base_entity_id, entity) > 0 && !!entity.Guid) {
					if(dlvrLocID) {
						if(tag_item.SetGuid(PPTAG_LOC_VETIS_GUID, &entity.Guid)) {
							THROW(p_ref->Ot.PutTag(PPOBJ_LOCATION, dlvrLocID, &tag_item, 0));
							ok = 1;
						}
					}
					else if(personID) {
						if(tag_item.SetGuid(PPTAG_PERSON_VETISUUID, &entity.Guid)) {
							THROW(p_ref->Ot.PutTag(PPOBJ_PERSON, personID, &tag_item, 0));
							ok = 1;
							is_psn_guid_set = 1;
						}
					}
				}
				if(!is_psn_guid_set && GetEntity(rec.ToEntityID, entity) > 0 && !!entity.Guid) {
					if(personID && tag_item.SetGuid(PPTAG_PERSON_VETISUUID, &entity.Guid)) {
						THROW(p_ref->Ot.PutTag(PPOBJ_PERSON, personID, &tag_item, 0));
						ok = 1;
					}
				}
				{
					const int idx = 7;
					VetisDocumentTbl::Key7 key;
					if(base_entity_id) {
						MEMSZERO(key);
						key.ToEnterpriseID = base_entity_id;
						if(DT.searchForUpdate(idx, &key, spGe) && DT.data.ToEnterpriseID == base_entity_id) do {
							if(DT.data.LinkToPsnID != personID || DT.data.LinkToDlvrLocID != dlvrLocID) {
								DT.data.LinkToPsnID = personID;
								DT.data.LinkToDlvrLocID = dlvrLocID;
								THROW_DB(DT.updateRec());
								ok = 1;
							}
						} while(DT.searchForUpdate(idx, &key, spNext) && DT.data.ToEnterpriseID == base_entity_id);
						{
							MEMSZERO(vp_key0);
							vp_key0.EntityID = base_entity_id;
							if(BT.searchForUpdate(0, &vp_key0, spEq)) {
								if(BT.data.LinkPersonID != personID || BT.data.LinkDlvrLocID != dlvrLocID) {
									BT.data.LinkPersonID = personID;
									BT.data.LinkDlvrLocID = dlvrLocID;
									THROW_DB(BT.updateRec());
									ok = 1;
								}
							}
						}
					}
				}
			}
		}
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI VetisEntityCore::Get(PPID id, VetisVetDocument & rItem)
{
	Reference * p_ref = PPRef;
	Entity entity;
	int    ok = GetEntity(id, entity);
	if(ok > 0) {
		SString temp_buf;
		SStringU temp_buf_u;
		VetisDocumentTbl::Rec rec;
		rItem.EntityID = id;
		rItem.Uuid = entity.Uuid;
		//VetisDocumentTbl::Key0 k0;
		//k0.EntityID = entity.ID;
		//if(DT.search(0, &k0, spEq)) {
		if(SearchDocument(entity.ID, &rec) > 0) {
			VetisCertifiedConsignment & r_crtc = rItem.CertifiedConsignment;
			Entity sub_entity;
			DT.copyBufTo(&rec);
			rItem.EntityID = rec.EntityID;
			rItem.Flags = rec.Flags;
			rItem.VetDForm = rec.VetisDocForm;
			rItem.VetDType = rec.VetisDocType;
			rItem.VetDStatus = rec.VetisDocStatus;
			rItem.IssueDate = rec.IssueDate;
			rItem.IssueNumber = rec.IssueNumber;
			rItem.WayBillDate = rec.WayBillDate;
			rItem.WayBillNumber = rec.WayBillNumber;
			r_crtc.Batch.DateOfProduction.FirstDate.FromInt64(rec.ManufDateFrom);
			r_crtc.Batch.DateOfProduction.SecondDate.FromInt64(rec.ManufDateTo);
			r_crtc.Batch.ExpiryDate.FirstDate.FromInt64(rec.ExpiryFrom);
			r_crtc.Batch.ExpiryDate.SecondDate.FromInt64(rec.ExpiryTo);
			r_crtc.Batch.NativeGoodsID = rec.LinkGoodsID;
			r_crtc.Batch.Volume = rec.Volume;
			r_crtc.Batch.PackingAmount = rec.PackingAmount;
			r_crtc.Batch.ProductType = rec.ProductType;
			r_crtc.TransportInfo.TransportType = rec.TranspType;
			r_crtc.TransportStorageType = rec.TranspStrgType;
			if(rec.FromEntityID) {
				GetEntity(rec.FromEntityID, sub_entity);
				sub_entity.Get(r_crtc.Consignor.BusinessEntity);
			}
			if(rec.FromEnterpriseID) {
				GetEntity(rec.FromEnterpriseID, sub_entity);
				sub_entity.Get(r_crtc.Consignor.Enterprise);
			}
			if(rec.ToEntityID) {
				GetEntity(rec.ToEntityID, sub_entity);
				sub_entity.Get(r_crtc.Consignee.BusinessEntity);
			}
			if(rec.ToEnterpriseID) {
				GetEntity(rec.ToEnterpriseID, sub_entity);
				sub_entity.Get(r_crtc.Consignee.Enterprise);
			}
			if(rec.ProductID) {
				GetEntity(rec.ProductID, sub_entity);
				sub_entity.Get(r_crtc.Batch.Product);
			}
			if(rec.SubProductID) {
				GetEntity(rec.SubProductID, sub_entity);
				sub_entity.Get(r_crtc.Batch.SubProduct);
			}
			if(rec.OrgCountryID) {
				GetEntity(rec.OrgCountryID, sub_entity);
				sub_entity.Get(r_crtc.Batch.Origin.Country);
			}
			if(rec.UOMID) {
				GetEntity(rec.UOMID, sub_entity);
				sub_entity.Get(r_crtc.Batch.Unit);
			}
			if(rec.ProductItemID) {
				GetEntity(rec.ProductItemID, sub_entity);
				sub_entity.Get(r_crtc.Batch.ProductItem);
			}
			else {
				TextRefIdent tri(PPOBJ_VETISENTITY, id, VetisEntityCore::txtprpProductItemName);
				p_ref->UtrC.Search(tri, temp_buf_u);
				if(temp_buf_u.Len()) {
					temp_buf_u.CopyToUtf8(temp_buf, 0);
					temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
					r_crtc.Batch.ProductItem.Name = temp_buf;
				}
			}
			{
				struct TranspNumTabEntry {
					int    TxtIdent;
					SString & R_Text;
				};
				/*non static!*/ const TranspNumTabEntry transp_num_tab[] = {
					{ txtprpTranspVNum, r_crtc.TransportInfo.TransportNumber.VehicleNumber },
					{ txtprpTranspTNum, r_crtc.TransportInfo.TransportNumber.TrailerNumber },
					{ txtprpTranspCNum, r_crtc.TransportInfo.TransportNumber.ContainerNumber },
					{ txtprpTranspWNum, r_crtc.TransportInfo.TransportNumber.WagonNumber },
					{ txtprpTranspSNam, r_crtc.TransportInfo.TransportNumber.ShipName },
					{ txtprpTranspFNum, r_crtc.TransportInfo.TransportNumber.FlightNumber }
				};
				for(uint tti = 0; tti < SIZEOFARRAY(transp_num_tab); tti++) {
					const TranspNumTabEntry & r_tt_entry = transp_num_tab[tti];
					TextRefIdent tri(PPOBJ_VETISENTITY, id, r_tt_entry.TxtIdent);
					p_ref->UtrC.Search(tri, temp_buf_u);
					if(temp_buf_u.Len()) {
						temp_buf_u.CopyToUtf8(temp_buf, 0);
						temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
						r_tt_entry.R_Text = temp_buf;
					}
				}
			}
			{
				TextRefIdent tri(PPOBJ_VETISENTITY, id, VetisEntityCore::txtprpGoodsCodeList);
				p_ref->UtrC.Search(tri, temp_buf_u);
				if(temp_buf_u.Len()) {
					temp_buf_u.CopyToUtf8(temp_buf, 0);
					temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
					r_crtc.Batch.ProductMarkingList.setBuf(temp_buf);
				}
			}
		}
		else
			ok = -2;
	}
	return ok;
}
	
int SLAPI VetisEntityCore::Put(PPID * pID, const VetisVetDocument & rItem, TSVector <UnresolvedEntity> * pUreList, int use_ta)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	SString temp_buf;
	SStringU temp_buf_u;
	PPIDArray by_inn_psn_list;
	PPIDArray id_list;
	VetisDocumentTbl::Rec rec;
	LocationTbl::Rec loc_rec;
	PersonTbl::Rec psn_rec;
	Entity entity(rItem);
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		THROW(SetEntity(entity, pUreList, 0));
		MEMSZERO(rec);
		{
			const VetisCertifiedConsignment & r_crtc = rItem.CertifiedConsignment;
			VetisBusinessEntity bent;
			rec.EntityID = entity.ID;
			rec.Flags    = rItem.Flags;
			rec.Flags &= ~(rItem.fFromMainOrg|rItem.fToMainOrg);
			rec.VetisDocForm = rItem.VetDForm;
			rec.VetisDocType = rItem.VetDType;
			rec.VetisDocStatus = rItem.VetDStatus;
			rec.IssueDate = rItem.IssueDate;
			STRNSCPY(rec.IssueNumber, rItem.IssueNumber);
			rec.WayBillDate = rItem.WayBillDate;
			STRNSCPY(rec.WayBillNumber, rItem.WayBillNumber);
			rec.ManufDateFrom = r_crtc.Batch.DateOfProduction.FirstDate.ToInt64();
			rec.ManufDateTo = r_crtc.Batch.DateOfProduction.SecondDate.ToInt64();
			rec.ExpiryFrom = r_crtc.Batch.ExpiryDate.FirstDate.ToInt64();
			rec.ExpiryTo = r_crtc.Batch.ExpiryDate.SecondDate.ToInt64();
			rec.LinkGoodsID = r_crtc.Batch.NativeGoodsID;
			rec.Volume = r_crtc.Batch.Volume;
			rec.PackingAmount = r_crtc.Batch.PackingAmount;
			{
				Entity sub_entity(kEnterprise, r_crtc.Consignor.Enterprise);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.FromEnterpriseID = sub_entity.ID;
				if(sub_entity.ID && !!sub_entity.Guid) {
					if(p_ref->Ot.SearchObjectsByGuid(PPOBJ_LOCATION, PPTAG_LOC_VETIS_GUID, sub_entity.Guid, &id_list) > 0) {
						assert(id_list.getCount());
						const PPID loc_id = id_list.get(0);
						if(PsnObj.LocObj.Fetch(loc_id, &loc_rec) > 0) {
							rec.LinkFromDlvrLocID = loc_id;
							if(loc_rec.Type == LOCTYP_ADDRESS && loc_rec.OwnerID)
								rec.LinkFromPsnID = loc_rec.OwnerID;
							else if(loc_rec.Type == LOCTYP_WAREHOUSE)
								rec.Flags |= rItem.fFromMainOrg;
						}
					}
					else if(p_ref->Ot.SearchObjectsByGuid(PPOBJ_PERSON, PPTAG_PERSON_VETISUUID, sub_entity.Guid, &id_list) > 0) {
						assert(id_list.getCount());
						rec.LinkFromPsnID = id_list.get(0);
					}
				}
			}
			{
				Entity sub_entity(kBusinessEntity, r_crtc.Consignor.BusinessEntity);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.FromEntityID = sub_entity.ID;
				if(!rec.LinkFromPsnID && !(rec.Flags & rItem.fFromMainOrg) && Get(sub_entity.ID, bent) > 0 && bent.Inn.NotEmpty()) {
					by_inn_psn_list.clear();
					PsnObj.GetListByRegNumber(PPREGT_TPID, 0, bent.Inn, id_list);
					for(uint i = 0; i < id_list.getCount(); i++) {
						const PPID _id = id_list.get(i);
						if(PsnObj.Fetch(_id, &psn_rec) > 0)
							by_inn_psn_list.add(_id);
					}
					if(by_inn_psn_list.getCount() == 1)
						rec.LinkFromPsnID = by_inn_psn_list.get(0);
				}
			}
			{
				Entity sub_entity(kEnterprise, r_crtc.Consignee.Enterprise);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.ToEnterpriseID = sub_entity.ID;
				if(sub_entity.ID && !!sub_entity.Guid) {
					if(p_ref->Ot.SearchObjectsByGuid(PPOBJ_LOCATION, PPTAG_LOC_VETIS_GUID, sub_entity.Guid, &id_list) > 0) {
						assert(id_list.getCount());
						const PPID loc_id = id_list.get(0);
						if(PsnObj.LocObj.Fetch(loc_id, &loc_rec) > 0) {
							rec.LinkToDlvrLocID = loc_id;
							if(loc_rec.Type == LOCTYP_ADDRESS && loc_rec.OwnerID)
								rec.LinkToPsnID = loc_rec.OwnerID;
							else if(loc_rec.Type == LOCTYP_WAREHOUSE)
								rec.Flags |= rItem.fToMainOrg;
						}
					}
					else if(p_ref->Ot.SearchObjectsByGuid(PPOBJ_PERSON, PPTAG_PERSON_VETISUUID, sub_entity.Guid, &id_list) > 0) {
						assert(id_list.getCount());
						rec.LinkToPsnID = id_list.get(0);
					}
				}
			}
			{
				Entity sub_entity(kBusinessEntity, r_crtc.Consignee.BusinessEntity);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.ToEntityID = sub_entity.ID;
				if(!rec.LinkToPsnID && !(rec.Flags & rItem.fToMainOrg) && Get(sub_entity.ID, bent) > 0 && bent.Inn.NotEmpty()) {
					by_inn_psn_list.clear();
					PsnObj.GetListByRegNumber(PPREGT_TPID, 0, bent.Inn, id_list);
					for(uint i = 0; i < id_list.getCount(); i++) {
						const PPID _id = id_list.get(i);
						if(PsnObj.Fetch(_id, &psn_rec) > 0)
							by_inn_psn_list.add(_id);
					}
					if(by_inn_psn_list.getCount() == 1)
						rec.LinkToPsnID = by_inn_psn_list.get(0);
				}
			}
			{
				Entity sub_entity(kProduct, r_crtc.Batch.Product);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.ProductID = sub_entity.ID;
			}
			{
				Entity sub_entity(kSubProduct, r_crtc.Batch.SubProduct);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.SubProductID = sub_entity.ID;
			}
			{
				Entity sub_entity(kProductItem, r_crtc.Batch.ProductItem);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.ProductItemID = sub_entity.ID;
			}
			{
				Entity sub_entity(kCountry, r_crtc.Batch.Origin.Country);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.OrgCountryID = sub_entity.ID;
			}
			{
				Entity sub_entity(kUOM, r_crtc.Batch.Unit);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.UOMID = sub_entity.ID;
			}
			rec.ProductType = (int16)r_crtc.Batch.ProductType;
			rec.TranspStrgType = (int16)r_crtc.TransportStorageType;
			rec.TranspType = (int16)r_crtc.TransportInfo.TransportType;
			{
				VetisDocumentTbl::Key0 k0;
				k0.EntityID = entity.ID;
				if(DT.searchForUpdate(0, &k0, spEq)) {
					if(memcmp(&DT.data, &rec, sizeof(rec)) != 0) {
						THROW_DB(DT.updateRecBuf(&rec)); // @sfu
					}
					else
						ok = -1;
				}
				else {
					THROW_DB(DT.insertRecBuf(&rec));
				}
			}
			if(!rec.ProductItemID) {
				temp_buf = r_crtc.Batch.ProductItem.Name;
				if(temp_buf.NotEmptyS()) {
					temp_buf_u.CopyFromUtf8(temp_buf.ToLower().Transf(CTRANSF_INNER_TO_UTF8));
					TextRefIdent tri(PPOBJ_VETISENTITY, entity.ID, txtprpProductItemName);
					int sr = p_ref->UtrC.SetText(tri, temp_buf_u, 0);
					THROW(sr);
					if(sr > 0 && ok < 0)
						ok = 1;
				}
			}
			{
				struct TranspNumTabEntry {
					int    TxtIdent;
					const  SString & R_Text;
				};
				/*non static!*/ const TranspNumTabEntry transp_num_tab[] = {
					{ txtprpTranspVNum, r_crtc.TransportInfo.TransportNumber.VehicleNumber },
					{ txtprpTranspTNum, r_crtc.TransportInfo.TransportNumber.TrailerNumber },
					{ txtprpTranspCNum, r_crtc.TransportInfo.TransportNumber.ContainerNumber },
					{ txtprpTranspWNum, r_crtc.TransportInfo.TransportNumber.WagonNumber },
					{ txtprpTranspSNam, r_crtc.TransportInfo.TransportNumber.ShipName },
					{ txtprpTranspFNum, r_crtc.TransportInfo.TransportNumber.FlightNumber }
				};
				for(uint tti = 0; tti < SIZEOFARRAY(transp_num_tab); tti++) {
					const TranspNumTabEntry & r_tt_entry = transp_num_tab[tti];
					temp_buf = r_tt_entry.R_Text;
					if(temp_buf.NotEmptyS()) {
						temp_buf_u.CopyFromUtf8(temp_buf.ToLower().Transf(CTRANSF_INNER_TO_UTF8));
						TextRefIdent tri(PPOBJ_VETISENTITY, entity.ID, r_tt_entry.TxtIdent);
						int sr = p_ref->UtrC.SetText(tri, temp_buf_u, 0);
						THROW(sr);
						if(sr > 0 && ok < 0)
							ok = 1;
					}
				}
			}
			if(r_crtc.Batch.ProductMarkingList.getCount()) {
				SString code_list_buf;
				for(uint ssp = 0; r_crtc.Batch.ProductMarkingList.get(&ssp, temp_buf);) {
					code_list_buf.CatDivIfNotEmpty(';', 0).Cat(temp_buf);
				}
				if(code_list_buf.NotEmptyS()) {
					temp_buf_u.CopyFromUtf8(code_list_buf.ToLower().Transf(CTRANSF_INNER_TO_UTF8));
					TextRefIdent tri(PPOBJ_VETISENTITY, entity.ID, txtprpGoodsCodeList);
					int sr = p_ref->UtrC.SetText(tri, temp_buf_u, 0);
					THROW(sr);
					if(sr > 0 && ok < 0)
						ok = 1;
				}
			}
		}
		THROW(tra.Commit());
	}
	CATCHZOK
	return ok;
}

int SLAPI VetisEntityCore::Get(PPID id, VetisBusinessEntity & rItem)
{
	rItem.Z();
	Reference * p_ref = PPRef;
	Entity entity;
	int    ok = GetEntity(id, entity);
	if(ok > 0) {
		SString temp_buf;
		SStringU temp_buf_u;
		VetisPersonTbl::Rec rec;
		VetisPersonTbl::Key0 k0;
		rItem.EntityID = id;
		rItem.Guid = entity.Guid;
		rItem.Uuid = entity.Uuid;
		rItem.Flags = entity.Flags;
		k0.EntityID = entity.ID;
		if(BT.search(0, &k0, spEq)) {
			Entity sub_entity;
			PiT.copyBufTo(&rec);
			rItem.Inn = rec.INN;
			rItem.Kpp = rec.KPP;
		}
		else
			ok = -2;
	}
	return ok;
}

int SLAPI VetisEntityCore::Put(PPID * pID, const VetisBusinessEntity & rItem, TSVector <UnresolvedEntity> * pUreList, int use_ta)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	SString temp_buf;
	SStringU temp_buf_u;
	VetisPersonTbl::Rec rec;
	Entity entity(kBusinessEntity, rItem);
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		THROW(SetEntity(entity, pUreList, 0));
		MEMSZERO(rec);
		{
			rec.EntityID = entity.ID;
			rec.EnterpriseType = rItem.Type;
			STRNSCPY(rec.INN, rItem.Inn);
			STRNSCPY(rec.KPP, rItem.Kpp);
			{
				Entity sub_entity(kCountry, rItem.JuridicalAddress.Country);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.CountryID = sub_entity.ID;
			}
			{
				Entity sub_entity(kRegion, rItem.JuridicalAddress.Region);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.RegionID = sub_entity.ID;
			}
			{
				VetisPersonTbl::Key0 k0;
				k0.EntityID = entity.ID;
				if(BT.searchForUpdate(0, &k0, spEq)) {
					VetisPersonTbl::Rec ex_rec;
					BT.copyBufTo(&ex_rec);
					if(ex_rec.INN[0] && !rec.INN[0])
						STRNSCPY(rec.INN, ex_rec.INN);
					if(ex_rec.KPP[0] && !rec.KPP[0])
						STRNSCPY(rec.KPP, ex_rec.KPP);
					if(ex_rec.LinkPersonID && !rec.LinkPersonID)
						rec.LinkPersonID = ex_rec.LinkPersonID;
					if(ex_rec.LinkDlvrLocID && !rec.LinkDlvrLocID)
						rec.LinkDlvrLocID = ex_rec.LinkDlvrLocID;
					if(memcmp(&BT.data, &rec, sizeof(rec)) != 0) {
						THROW_DB(BT.updateRecBuf(&rec)); // @sfu
					}
					else
						ok = -1;
				}
				else {
					THROW_DB(BT.insertRecBuf(&rec));
				}
			}
			{
				temp_buf = rItem.JuridicalAddress.AddressView;
				if(temp_buf.NotEmptyS()) {
					temp_buf_u.CopyFromUtf8(temp_buf.ToLower().Transf(CTRANSF_INNER_TO_UTF8));
					TextRefIdent tri(PPOBJ_VETISENTITY, entity.ID, PPTRPROP_RAWADDR);
					int sr = p_ref->UtrC.SetText(tri, temp_buf_u, 0);
					THROW(sr);
					if(sr > 0 && ok < 0)
						ok = 1;
				}
			}
		}
		THROW(tra.Commit());
		ASSIGN_PTR(pID, entity.ID);
	}
	CATCHZOK
	return ok;
}
	
int SLAPI VetisEntityCore::Put(PPID * pID, const VetisEnterprise & rItem, TSVector <UnresolvedEntity> * pUreList, int use_ta)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	SString temp_buf;
	SStringU temp_buf_u;
	VetisPersonTbl::Rec rec;
	Entity entity(kEnterprise, rItem);
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		THROW(SetEntity(entity, pUreList, 0));
		MEMSZERO(rec);
		{
			rec.EntityID = entity.ID;
			rec.EnterpriseType = rItem.Type;
			{
				Entity sub_entity(kCountry, rItem.Address.Country);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.CountryID = sub_entity.ID;
			}
			{
				Entity sub_entity(kRegion, rItem.Address.Region);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.RegionID = sub_entity.ID;
			}
			{
				VetisPersonTbl::Key0 k0;
				k0.EntityID = entity.ID;
				if(BT.searchForUpdate(0, &k0, spEq)) {
					VetisPersonTbl::Rec ex_rec;
					BT.copyBufTo(&ex_rec);
					if(ex_rec.INN[0] && !rec.INN[0])
						STRNSCPY(rec.INN, ex_rec.INN);
					if(ex_rec.KPP[0] && !rec.KPP[0])
						STRNSCPY(rec.KPP, ex_rec.KPP);
					if(ex_rec.LinkPersonID && !rec.LinkPersonID)
						rec.LinkPersonID = ex_rec.LinkPersonID;
					if(ex_rec.LinkDlvrLocID && !rec.LinkDlvrLocID)
						rec.LinkDlvrLocID = ex_rec.LinkDlvrLocID;
					if(memcmp(&BT.data, &rec, sizeof(rec)) != 0) {
						THROW_DB(BT.updateRecBuf(&rec)); // @sfu
					}
					else
						ok = -1;
				}
				else {
					THROW_DB(BT.insertRecBuf(&rec));
				}
			}
			{
				temp_buf = rItem.Address.AddressView;
				if(temp_buf.NotEmptyS()) {
					temp_buf_u.CopyFromUtf8(temp_buf.ToLower().Transf(CTRANSF_INNER_TO_UTF8));
					TextRefIdent tri(PPOBJ_VETISENTITY, entity.ID, PPTRPROP_RAWADDR);
					int sr = p_ref->UtrC.SetText(tri, temp_buf_u, 0);
					THROW(sr);
					if(sr > 0 && ok < 0)
						ok = 1;
				}
			}
		}
		THROW(tra.Commit());
		ASSIGN_PTR(pID, entity.ID);
	}
	CATCHZOK
	return ok;
}

int SLAPI VetisEntityCore::Get(PPID id, VetisProductItem & rItem)
{
	Reference * p_ref = PPRef;
	Entity entity;
	int    ok = GetEntity(id, entity);
	if(ok > 0) {
		SString temp_buf;
		SStringU temp_buf_u;
		VetisProductTbl::Rec rec;
		VetisProductTbl::Key0 k0;
		rItem.EntityID = id;
		rItem.Guid = entity.Guid;
		rItem.Uuid = entity.Uuid;
		rItem.Flags = entity.Flags;
		k0.EntityID = entity.ID;
		if(PiT.search(0, &k0, spEq)) {
			Entity sub_entity;
			PiT.copyBufTo(&rec);
			rItem.NativeGoodsID = rec.LinkGoodsID;
			rItem.ProductType = rec.ProductType;
			rItem.GlobalID = rec.GTIN;
			if(rec.ProductID) {
				GetEntity(rec.ProductID, sub_entity);
				sub_entity.Get(rItem.Product);
			}
			if(rec.SubProductID) {
				GetEntity(rec.SubProductID, sub_entity);
				sub_entity.Get(rItem.SubProduct);
			}
			if(rec.GostTRef && p_ref->TrT.SearchSelfRef(rec.GostTRef, temp_buf_u) > 0) {
				temp_buf_u.CopyToUtf8(temp_buf, 1);
				temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
				rItem.Gost = temp_buf;
			}
		}
		else
			ok = -2;
	}
	return ok;
}
    
int SLAPI VetisEntityCore::Put(PPID * pID, int kind, const VetisProductItem & rItem, TSVector <UnresolvedEntity> * pUreList, int use_ta)
{
	int    ok = 1;
	Reference * p_ref = PPRef;
	SString temp_buf;
	SStringU temp_buf_u;
	VetisProductTbl::Rec rec;
	Entity entity(kind, rItem);
	assert(oneof3(kind, kProductItem, kProduct, kSubProduct));
	THROW_PP(oneof3(kind, kProductItem, kProduct, kSubProduct), PPERR_INVPARAM);
	{
		PPTransaction tra(use_ta);
		THROW(tra);
		THROW(SetEntity(entity, pUreList, 0));
		MEMSZERO(rec);
		rec.EntityID = entity.ID;
		if(kind == kProductItem) {
			temp_buf = rItem.Gost;
			if(temp_buf.NotEmptyS()) {
				temp_buf_u.CopyFromUtf8(temp_buf.Trim(124).ToLower().Transf(CTRANSF_INNER_TO_UTF8));
				THROW(p_ref->TrT.GetSelfRefText(temp_buf_u, &rec.GostTRef, 0));
			}
		}
		else if(kind == kProduct) {
		}
		else if(kind == kSubProduct) {
		}
		rec.ProductType = rItem.ProductType;
		if(kind == kProductItem) {
			rec.LinkGoodsID = rItem.NativeGoodsID;
			{
				Entity sub_entity(kProduct, rItem);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.ProductID = sub_entity.ID;
			}
			{
				Entity sub_entity(kSubProduct, rItem);
				THROW(SetEntity(sub_entity, pUreList, 0));
				rec.SubProductID = sub_entity.ID;
			}
			STRNSCPY(rec.GTIN, rItem.GlobalID);
			{
				VetisProductTbl::Key0 k0;
				k0.EntityID = entity.ID;
				if(PiT.searchForUpdate(0, &k0, spEq)) {
					if(memcmp(&PiT.data, &rec, sizeof(rec)) != 0) {
						THROW_DB(PiT.updateRecBuf(&rec)); // @sfu
					}
					else
						ok = -1;
				}
				else {
					THROW_DB(PiT.insertRecBuf(&rec));
				}
			}
		}
		THROW(tra.Commit());
		ASSIGN_PTR(pID, entity.ID);
	}
	CATCHZOK
	return ok;
}

int SLAPI VetisEntityCore::Get(PPID id, VetisEnterprise & rItem)
{
	int    ok = -1;
	return ok;
}

int SLAPI VetisEntityCore::Get(PPID id, VetisProduct & rItem)
{
	int    ok = -1;
	return ok;
}

int SLAPI VetisEntityCore::Get(PPID id, VetisSubProduct & rItem)
{
	int    ok = -1;
	return ok;
}
	
int SLAPI VetisEntityCore::RecToItem(const VetisProductTbl::Rec & rRec, VetisProductItem & rItem)
{
	int    ok = -1;
	return ok;
}

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
		SLAPI  Param(PPID mainOrgID, PPID locID) : MainOrgID(mainOrgID), LocID(locID)
		{
		}
		void   Clear()
		{
			SetBuffer(0);
			IssuerUUID.Z();
			EntUUID.Z();
		}
		PPID   MainOrgID;
		PPID   LocID;
		S_GUID IssuerUUID; // GUID локации, генерурующей запросы
		S_GUID EntUUID; // GUID предприятия
	};
	explicit SLAPI PPVetisInterface(PPLogger * pLogger);
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
	int    SLAPI GetVetDocumentList(uint startOffset, uint count, VetisApplicationBlock & rReply);
	int    SLAPI GetVetDocumentChangesList(const STimeChunk & rPeriod, uint startOffset, uint count, VetisApplicationBlock & rReply);
	int    SLAPI GetVetDocumentByUuid(const S_GUID & rUuid, VetisApplicationBlock & rReply);
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
	int    SLAPI ProcessIncomingConsignment(const S_GUID & rDocUuid, VetisApplicationBlock & rReply);
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

	int    SLAPI GetRussianEnterpriseList(uint offs, uint count, VetisApplicationBlock & rReply);
	int    SLAPI GetBusinessEntityList(uint offs, uint count, VetisApplicationBlock & rReply);
	int    SLAPI GetProductItemList(uint offs, uint count, VetisApplicationBlock & rReply);

	enum {
		qtProductItemByGuid = 1,
		qtProductItemByUuid,
		qtProductByGuid,
		qtProductByUuid,
		qtSubProductByGuid,
		qtSubProductByUuid,
		qtBusinessEntityByGuid,
		qtBusinessEntityByUuid,
		qtEnterpriseByGuid,
		qtEnterpriseByUuid
	};

	int    SLAPI GetEntityQuery(int queryType, const char * pQueryParam, VetisApplicationBlock & rReply);
	static int SLAPI SetupParam(Param & rP);

	int    SLAPI ProcessUnresolvedEntityList(TSVector <VetisEntityCore::UnresolvedEntity> & rList);

	VetisEntityCore PeC;
private:
	class VetisSubmitRequestBlock : public SXmlWriter {
	public:
		VetisSubmitRequestBlock();
		~VetisSubmitRequestBlock();
	private:
		SXml::WDoc D;
	};
	int    SLAPI PrepareApplicationBlockForReq(VetisApplicationBlock & rBlk);
	int    SLAPI SubmitRequest(VetisApplicationBlock & rAppBlk, VetisApplicationBlock & rResult);
	int    ReceiveResult(const S_GUID & rAppId, VetisApplicationBlock & rResult);
	//int    SLAPI PrepareAppReqData(VetisApplicationBlock & rBlk, const void * pAppData);
	int    SLAPI SendSOAP(const char * pUrl, const char * pAction, const SString & rPack, SString & rReply);
	int    SLAPI MakeAuthField(SString & rBuf);
	int    SLAPI ParseReply(const SString & rReply, VetisApplicationBlock & rResult);
	int    SLAPI ParseError(xmlNode * pNode, VetisErrorEntry & rResult);
	int    SLAPI ParseFault(xmlNode * pParentNode, VetisFault & rResult);
	//int    SLAPI ParseApplicationBlock(xmlNode * pParentNode, VetisApplicationBlock & rResult);
	int    SLAPI ParseDocument(xmlNode * pParentNode, VetisDocument & rResult);
	int    SLAPI ParseVetDocument(xmlNode * pParentNode, VetisVetDocument & rResult);
	int    SLAPI ParseCertifiedConsignment(xmlNode * pParentNode, VetisCertifiedConsignment & rResult);
	int    SLAPI ParseLocality(xmlNode * pParentNode, VetisLocality & rResult);
	int    SLAPI ParseCountry(xmlNode * pParentNode, VetisCountry & rResult);
	int    SLAPI ParseRegion(xmlNode * pParentNode, VetisAddressObjectView & rResult);
	int    SLAPI ParseAddress(xmlNode * pParentNode, VetisAddress & rResult);
	int    SLAPI ParseBusinessMember(xmlNode * pParentNode, VetisBusinessMember & rResult);
	int    SLAPI ParseEnterprise(xmlNode * pParentNode, VetisEnterprise & rResult);
	int    SLAPI ParseProducer(xmlNode * pParentNode, VetisProducer & rResult);
	int    SLAPI ParseBusinessEntity(xmlNode * pParentNode, VetisBusinessEntity & rResult);
	int    SLAPI ParseNamedGenericVersioningEntity(xmlNode * pParentNode, VetisNamedGenericVersioningEntity & rResult);
	int    SLAPI ParseBatch(xmlNode * pParentNode, VetisBatch & rResult);
	int    SLAPI ParseProduct(xmlNode * pParentNode, VetisProduct & rResult);
	int    SLAPI ParseSubProduct(xmlNode * pParentNode, VetisSubProduct & rResult);
	int    SLAPI ParseProductItem(xmlNode * pParentNode, VetisProductItem & rResult);
	int    SLAPI ParseComplexDate(xmlNode * pParentNode, SUniTime & rResult);
	int    SLAPI ParseGoodsDate(xmlNode * pParentNode, VetisGoodsDate & rResult);
	int    SLAPI ParseUnit(xmlNode * pParentNode, VetisUnit & rResult);
	int    SLAPI ParseTransportInfo(xmlNode * pParentNode, VetisTransportInfo & rResult);
	void   SLAPI ParseListResult(const xmlNode * pNode, VetisApplicationBlock::ReplyListValues & rResult);
	void   SLAPI PutListOptionsParam(xmlTextWriter * pWriter, uint offs, uint count);
	int    SLAPI PutGoodsDate(xmlTextWriter * pWriter, const char * pScopeXmlTag, const SUniTime & rUt);

	int    SLAPI LogMessage(const char * pPrefix, const SString & rMsg);
	int    SLAPI LogFaults(const VetisApplicationBlock & rAb);

	long   State;
	SString LogFileName;
	SString LastMsg;
	Param   P;
	int64   LastLocalTransactionId;
	PPLogger * P_Logger;
	PPObjGoods GObj;
};

int SLAPI PPVetisInterface::LogMessage(const char * pPrefix, const SString & rMsg)
{
	int    ok = 1;
	SString file_name;
	PPGetFilePath(PPPATH_LOG, "vetis-msg.log", file_name);
	SFile  f_log(file_name, SFile::mAppend);
	if(f_log.IsValid()) {
		if(!isempty(pPrefix)) {
			SString temp_buf;
			temp_buf.Cat(getcurdatetime_(), DATF_YMD|DATF_CENTURY, TIMF_HMS|TIMF_MSEC).Space().Cat(pPrefix).CR();
			f_log.WriteLine(temp_buf);
		}
		f_log.WriteLine(rMsg);
		f_log.WriteLine(0);
	}
	else
		ok = 0;
	return ok;
}

//static 
int SLAPI PPVetisInterface::SetupParam(Param & rP)
{
	rP.Clear();

	int    ok = 1;
	Reference * p_ref = PPRef;
	SString temp_buf;
	PPAlbatrosConfig acfg;
	THROW(PPAlbatrosCfgMngr::Get(&acfg) > 0);
	if(!rP.MainOrgID) {
		GetMainOrgID(&rP.MainOrgID);
	}
	if(!rP.LocID) {
		rP.LocID = LConfig.Location;
	}
	acfg.GetExtStrData(ALBATROSEXSTR_VETISUSER, temp_buf);
	THROW_PP(temp_buf.NotEmptyS(), PPERR_VETISUSERUNDEF);
	rP.PutExtStrData(extssUser, temp_buf);
	acfg.GetPassword(ALBATROSEXSTR_VETISPASSW, temp_buf);
	THROW_PP(temp_buf.NotEmptyS(), PPERR_VETISUSERUNDEF);
	rP.PutExtStrData(extssPassword, temp_buf);
	acfg.GetExtStrData(ALBATROSEXSTR_VETISAPIKEY, temp_buf);
	THROW_PP(temp_buf.NotEmptyS(), PPERR_VETISAPIKEYUNDEF);
	rP.PutExtStrData(extssApiKey, temp_buf);
	{
		ObjTagItem tag_item;
		if(p_ref->Ot.GetTag(PPOBJ_PERSON, rP.MainOrgID, PPTAG_PERSON_VETISUUID, &tag_item) > 0) {
			tag_item.GetGuid(&rP.IssuerUUID);
		}
		GetPersonName(rP.MainOrgID, temp_buf);
		THROW_PP_S(rP.IssuerUUID, PPERR_VETISBUSENTGUIDUNDEF, temp_buf);
		//
		if(p_ref->Ot.GetTag(PPOBJ_LOCATION, rP.LocID, PPTAG_LOC_VETIS_GUID, &tag_item) > 0) {
			tag_item.GetGuid(&rP.EntUUID);
		}
		GetLocationName(rP.LocID, temp_buf);
		THROW_PP_S(rP.EntUUID, PPERR_VETISLOCGUIDUNDEF, temp_buf);
		//
		if(p_ref->Ot.GetTag(PPOBJ_PERSON, rP.MainOrgID, PPTAG_PERSON_VETISUSER, &tag_item) > 0) {
			tag_item.GetStr(temp_buf);
			rP.PutExtStrData(extssQInitiator, temp_buf);
		}
	}
	CATCHZOK
	return ok;
}

SLAPI PPVetisInterface::PPVetisInterface(PPLogger * pLogger) : State(0), LastLocalTransactionId(0), P_Logger(pLogger), P(0, 0)
{
	PPGetFilePath(PPPATH_LOG, "vetis.log", LogFileName);
}

SLAPI PPVetisInterface::~PPVetisInterface()
{
}

int SLAPI PPVetisInterface::Init(const Param & rP)
{
	int    ok = 1;
	P = rP;
	State |= stInited;
	return ok;
}

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
	//static const char * P_VetisSoapUrl = "https://api.vetrf.ru/platform/services/ApplicationManagementService"; // product
	//static const char * P_VetisSoapUrl = "https://api2.vetrf.ru:8002/platform/services/ApplicationManagementService"; // test
	rReply.Z();
	int    ok = -1;
	SString temp_buf;
	ScURL  c;
	const char * p_url = pUrl ? pUrl : InetUrl::MkHttps("api.vetrf.ru", "platform/services/ApplicationManagementService"); 
	InetUrl url(p_url);
	p_url = 0; // возможно, указывает на револьверный буфер - лучше обнулить во избежании использования //
	StrStrAssocArray hdr_flds;
	SBuffer ack_buf;
	SFile wr_stream(ack_buf, SFile::mWrite);
	{
		SFileFormat::GetMime(SFileFormat::Xml, temp_buf);
		temp_buf.CatDiv(';', 2).CatEq("charset", "utf-8");
		SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrContentType, temp_buf);
		SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrContentLen, temp_buf.Z().Cat(rPack.Len()));
		THROW(MakeAuthField(temp_buf));
		SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrAuthorization, temp_buf);
		if(!isempty(pAction)) {
			SHttpProtocol::SetHeaderField(hdr_flds, SHttpProtocol::hdrSoapAction, temp_buf.CatQStr(pAction));
		}
	}
	LogMessage("query", rPack);
	THROW_SL(c.HttpPost(url, ScURL::mfDontVerifySslPeer|ScURL::mfVerbose, &hdr_flds, rPack, &wr_stream));
	{
		SBuffer * p_ack_buf = (SBuffer *)wr_stream;
		if(p_ack_buf) {
			const int avl_size = (int)p_ack_buf->GetAvailableSize();
			rReply.Z().CatN((const char *)p_ack_buf->GetBuf(), avl_size);
			LogMessage("reply", rReply);
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
		VetisErrorEntry single_err;
		for(xmlNode * p_i2 = pParentNode->children; p_i2; p_i2 = p_i2->next) {
			if(SXml::GetContentByName(p_i2, "message", temp_buf))
				rResult.Message = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
			else if(SXml::GetContentByName(p_i2, "errorCode", temp_buf))
				single_err.Code = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
			else if(SXml::GetContentByName(p_i2, "reason", temp_buf))
				single_err.Item = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
			else if(SXml::GetContentByName(p_i2, "location", temp_buf)) {
			}
			else if(ParseError(p_i2, err) > 0) {
				VetisErrorEntry * p_err = rResult.ErrList.CreateNewItem();
				THROW_SL(p_err);
				*p_err = err;
			}
		}
		if(!single_err.IsEmpty()) {
			VetisErrorEntry * p_err = rResult.ErrList.CreateNewItem();
			THROW_SL(p_err);
			*p_err = single_err;
		}
	}
	CATCHZOK
	return ok;
}

static const SIntToSymbTabEntry VetisAppStatus_SymbTab[] = {
	{ VetisApplicationBlock::appstAccepted, "Accepted" },
	{ VetisApplicationBlock::appstRejected, "Rejected" },
	{ VetisApplicationBlock::appstCompleted, "Completed" },
	{ VetisApplicationBlock::appstInProcess, "InProcess" },
	{ VetisApplicationBlock::appstInProcess, "IN_PROCESS" },
};

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

static const SIntToSymbTabEntry VetisTranspStorageType_SymbTab[] = {
	{ vtstUNDEF, "UNDEFINED" },
	{ vtstFROZEN, "FROZEN" },
	{ vtstCHILLED, "CHILLED" },
	{ vtstCOOLED, "COOLED" },
	{ vtstVENTILATED, "VENTILATED" }
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
		else if(SXml::GetContentByName(p_a, "active", temp_buf)) {
			SETFLAG(rResult.Flags, rResult.fActive, temp_buf.IsEqiAscii("true"));
		}
		else if(SXml::GetContentByName(p_a, "last", temp_buf)) {
			SETFLAG(rResult.Flags, rResult.fLast, temp_buf.IsEqiAscii("true"));
		}
		else if(SXml::GetContentByName(p_a, "status", temp_buf))
			rResult.Status = temp_buf.ToLong();
		else if(SXml::GetContentByName(p_a, "createDate", temp_buf))
			strtodatetime(temp_buf, &rResult.CreateDate, DATF_ISO8601, TIMF_HMS);
		else if(SXml::GetContentByName(p_a, "updateDate", temp_buf))
			strtodatetime(temp_buf, &rResult.UpdateDate, DATF_ISO8601, TIMF_HMS);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseLocality(xmlNode * pParentNode, VetisLocality & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "regionGUID", temp_buf))
			rResult.RegionGUID.FromStr(temp_buf);
		else if(SXml::GetContentByName(p_a, "districtGUID", temp_buf))
			rResult.DistrictGUID.FromStr(temp_buf);
		else if(SXml::GetContentByName(p_a, "cityGUID", temp_buf))
			rResult.CityGUID.FromStr(temp_buf);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseCountry(xmlNode * pParentNode, VetisCountry & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "fullName", temp_buf))
			rResult.FullName = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		else if(SXml::GetContentByName(p_a, "englishName", temp_buf))
			rResult.EnglishName = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseRegion(xmlNode * pParentNode, VetisAddressObjectView & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "englishName", temp_buf))
			rResult.EnglishName = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseAddress(xmlNode * pParentNode, VetisAddress & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "country")) {
		}
		else if(SXml::IsName(p_a, "region")) {
		}
		else if(SXml::GetContentByName(p_a, "district", temp_buf)) {
		}
		else if(SXml::GetContentByName(p_a, "locality", temp_buf)) {
			ParseLocality(p_a, rResult.Locality);
		}
		else if(SXml::GetContentByName(p_a, "subLocality", temp_buf)) {
			ParseLocality(p_a, rResult.SubLocality);
		}
		else if(SXml::GetContentByName(p_a, "street", temp_buf)) {
		}
		else if(SXml::GetContentByName(p_a, "addressView", temp_buf))
			rResult.AddressView = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseEnterprise(xmlNode * pParentNode, VetisEnterprise & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "type", temp_buf)) {
		}
		else if(SXml::IsName(p_a, "address")) {
			ParseAddress(p_a, rResult.Address);
		}
		else if(SXml::IsName(p_a, "activityList")) {
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseProducer(xmlNode * pParentNode, VetisProducer & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "enterprise")) {
			ParseEnterprise(p_a, rResult);
		}
		else if(SXml::GetContentByName(p_a, "role", temp_buf)) {
			if(temp_buf.IsEqiAscii("PRODUCER"))
				rResult.Role = rResult.rolePRODUCER;
			else 
				rResult.Role = rResult.roleUNKNOWN;
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseBusinessEntity(xmlNode * pParentNode, VetisBusinessEntity & rResult)
{
	int    ok = 1;
	SString temp_buf;
	SString firm_name;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "fio", temp_buf)) {
			rResult.Fio = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		else if(SXml::GetContentByName(p_a, "firmName", temp_buf)) {
			firm_name = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		else if(SXml::GetContentByName(p_a, "inn", temp_buf)) {
			rResult.Inn = temp_buf;
		}
		else if(SXml::GetContentByName(p_a, "kpp", temp_buf)) {
			rResult.Kpp = temp_buf;
		}
		else if(SXml::GetContentByName(p_a, "passport", temp_buf)) {
			rResult.Passport = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		else if(SXml::IsName(p_a, "country")) {
		}
		else if(SXml::IsName(p_a, "region")) {
		}
		else if(SXml::IsName(p_a, "actualRegion")) {
		}
		else if(SXml::IsName(p_a, "juridicalAddress")) {
		}
		else if(SXml::IsName(p_a, "actualAddress")) {
		}
		else if(SXml::IsName(p_a, "activityLocation")) {
		}
	}
	if(rResult.Name.Empty()) {
		if(firm_name.NotEmptyS())
			rResult.Name = firm_name;
		else if(rResult.Fio.NotEmpty())
			rResult.Name = rResult.Fio;
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseBusinessMember(xmlNode * pParentNode, VetisBusinessMember & rResult)
{
	int    ok = 1;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "businessEntity"))
			ParseBusinessEntity(p_a, rResult.BusinessEntity);
		else if(SXml::IsName(p_a, "enterprise"))
			ParseEnterprise(p_a, rResult.Enterprise);
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
		else if(SXml::IsName(p_a, "code"))
			rResult.Code = temp_buf;
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseSubProduct(xmlNode * pParentNode, VetisSubProduct & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::IsName(p_a, "code"))
			rResult.Code = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseProductItem(xmlNode * pParentNode, VetisProductItem & rResult)
{
	int    ok = 1;
	SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "correspondsToGost", temp_buf)) {
			SETFLAG(rResult.Flags, rResult.fCorrespondsToGost, temp_buf.IsEqiAscii("true"));
		}
		else if(SXml::GetContentByName(p_a, "productType", temp_buf))
			rResult.ProductType = temp_buf.ToLong();
		else if(SXml::GetContentByName(p_a, "gost", temp_buf))
			rResult.Gost = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		else if(SXml::GetContentByName(p_a, "product", temp_buf)) {
			THROW(ParseProduct(p_a, rResult.Product));
		}
		else if(SXml::GetContentByName(p_a, "subProduct", temp_buf)) {
			THROW(ParseSubProduct(p_a, rResult.SubProduct));
		}
		else if(SXml::GetContentByName(p_a, "producing", temp_buf)) {
		}
		else if(SXml::GetContentByName(p_a, "globalID", temp_buf)) {
			STokenRecognizer tr;
			SNaturalTokenArray nta;
			tr.Run(temp_buf.ucptr(), temp_buf.Len(), nta.Z(), 0);
			if(nta.Has(SNTOK_EAN13) || nta.Has(SNTOK_EAN8) || nta.Has(SNTOK_UPCE) || nta.Has(SNTOK_UPCA)) {
				rResult.GlobalID = temp_buf;
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::ParseUnit(xmlNode * pParentNode, VetisUnit & rResult)
{
	int    ok = 1;
	//SString temp_buf;
	ParseNamedGenericVersioningEntity(pParentNode, rResult);
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		;
	}
	//CATCHZOK
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
	SString attr_buf;
	STokenRecognizer tr;
	SNaturalTokenArray nta;
	rResult.Flags &= ~rResult.fPerishable;
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
		else if(SXml::GetContentByName(p_a, "packingAmount", temp_buf))
			rResult.PackingAmount = temp_buf.ToLong();	
		else if(SXml::IsName(p_a, "unit"))
			ParseUnit(p_a, rResult.Unit);
		else if(SXml::IsName(p_a, "dateOfProduction"))
			ParseGoodsDate(p_a, rResult.DateOfProduction);
		else if(SXml::IsName(p_a, "expiryDate"))
			ParseGoodsDate(p_a, rResult.ExpiryDate);
		else if(SXml::GetContentByName(p_a, "perishable", temp_buf)) {
			if(temp_buf.IsEqiAscii("true"))
				rResult.Flags |= rResult.fPerishable;
		}
		else if(SXml::IsName(p_a, "countryOfOrigin"))
			ParseCountry(p_a, rResult.Origin.Country);
		else if(SXml::IsName(p_a, "packingList")) {
			for(xmlNode * p_p = p_a->children; p_p; p_p = p_p->next) {
				if(SXml::IsName(p_p, "packingForm")) {
					VetisNamedGenericVersioningEntity * p_new_pf = rResult.PackingList.CreateNewItem();
					THROW_SL(p_new_pf);
					ParseNamedGenericVersioningEntity(p_p, *p_new_pf);
				}
			}
		}
		else if(SXml::IsName(p_a, "producerList")) {
			for(xmlNode * p_p = p_a->children; p_p; p_p = p_p->next) {
				if(SXml::IsName(p_p, "producer")) {
					VetisProducer * p_new_p = rResult.Origin.Producer.CreateNewItem();
					THROW_SL(p_new_p);
					ParseProducer(p_p, *p_new_p);
				}
			}
		}
		else if(SXml::IsName(p_a, "productMarkingList")) {
			/*
				<ns2:productMarkingList>
					<ns2:productMarking>4607084550559</ns2:productMarking>
					<ns2:productMarking class="BN">X167</ns2:productMarking>
				</ns2:productMarkingList>
			*/
			for(xmlNode * p_p = p_a->children; p_p; p_p = p_p->next) {
				if(SXml::GetContentByName(p_p, "productMarking", temp_buf)) {
					if(SXml::GetAttrib(p_p, "class", attr_buf)) {
						;
					}
					else {
						tr.Run(temp_buf.ucptr(), temp_buf.Len(), nta.Z(), 0);
						if(nta.Has(SNTOK_EAN13) || nta.Has(SNTOK_EAN8) || nta.Has(SNTOK_UPCE) || nta.Has(SNTOK_UPCA)) {
							rResult.ProductMarkingList.add(temp_buf);
						}
					}
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::ParseTransportInfo(xmlNode * pParentNode, VetisTransportInfo & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "transportType", temp_buf)) {
			rResult.TransportType = temp_buf.ToLong();
		}
		else if(SXml::IsName(p_a, "transportNumber")) {
			for(xmlNode * p_tn = p_a->children; p_tn; p_tn = p_tn->next) {
				if(SXml::GetContentByName(p_tn, "vehicleNumber", temp_buf))
					rResult.TransportNumber.VehicleNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER).Strip();
				else if(SXml::GetContentByName(p_tn, "trailerNumber", temp_buf))
					rResult.TransportNumber.TrailerNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER).Strip();
				else if(SXml::GetContentByName(p_tn, "containerNumber", temp_buf))
					rResult.TransportNumber.ContainerNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER).Strip();
				else if(SXml::GetContentByName(p_tn, "wagonNumber", temp_buf))
					rResult.TransportNumber.WagonNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER).Strip();
				else if(SXml::GetContentByName(p_tn, "flightNumber", temp_buf))
					rResult.TransportNumber.FlightNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER).Strip();
				else if(SXml::GetContentByName(p_tn, "shipName", temp_buf))
					rResult.TransportNumber.ShipName = temp_buf.Transf(CTRANSF_UTF8_TO_INNER).Strip();
			}
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseCertifiedConsignment(xmlNode * pParentNode, VetisCertifiedConsignment & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		int    r = 1;
		if(SXml::IsName(p_a, "consignor"))
			r = ParseBusinessMember(p_a, rResult.Consignor);
		else if(SXml::IsName(p_a, "consignee"))
			r = ParseBusinessMember(p_a, rResult.Consignee);
		else if(SXml::IsName(p_a, "broker"))
			r = ParseBusinessEntity(p_a, rResult.Broker);
		else if(SXml::IsName(p_a, "batch"))
			r = ParseBatch(p_a, rResult.Batch);
		else if(SXml::IsName(p_a, "transportInfo"))
			r = ParseTransportInfo(p_a, rResult.TransportInfo);
		else if(SXml::GetContentByName(p_a, "transportStorageType", temp_buf))
			rResult.TransportStorageType = SIntToSymbTab_GetId(VetisTranspStorageType_SymbTab, SIZEOFARRAY(VetisTranspStorageType_SymbTab), temp_buf);
		THROW(r);
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::ParseDocument(xmlNode * pParentNode, VetisDocument & rResult)
{
	int    ok = 1;
	SString temp_buf;
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "uuid", temp_buf))
			rResult.Uuid.FromStr(temp_buf);
		else if(SXml::GetContentByName(p_a, "name", temp_buf))
			rResult.Name = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		else if(SXml::GetContentByName(p_a, "form", temp_buf))
			rResult.DocumentForm = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		else if(SXml::GetContentByName(p_a, "issueDate", temp_buf))
			rResult.IssueDate = strtodate_(temp_buf, DATF_ISO8601);
		else if(SXml::GetContentByName(p_a, "issueNumber", temp_buf))
			rResult.IssueNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		else if(SXml::GetContentByName(p_a, "issueSeries", temp_buf))
			rResult.IssueSeries = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		else if(SXml::IsName(p_a, "purpose")) {
		}
		else if(SXml::IsName(p_a, "confirmedBy")) {
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ParseVetDocument(xmlNode * pParentNode, VetisVetDocument & rResult)
{
	int    ok = 1;
	SString temp_buf;
	VetisCertifiedConsignment & r_crtc = rResult.CertifiedConsignment;
	THROW(ParseDocument(pParentNode, rResult));
	THROW(ParseCertifiedConsignment(pParentNode, r_crtc));
	for(xmlNode * p_a = pParentNode ? pParentNode->children : 0; p_a; p_a = p_a->next) {
		if(SXml::GetContentByName(p_a, "form", temp_buf))
			rResult.VetDForm = SIntToSymbTab_GetId(VetisVetDocFormat_SymbTab, SIZEOFARRAY(VetisVetDocFormat_SymbTab), temp_buf);
		else if(SXml::GetContentByName(p_a, "type", temp_buf))
			rResult.VetDType = SIntToSymbTab_GetId(VetisVetDocType_SymbTab, SIZEOFARRAY(VetisVetDocType_SymbTab), temp_buf);
		else if(SXml::GetContentByName(p_a, "status", temp_buf))
			rResult.VetDStatus = SIntToSymbTab_GetId(VetisVetDocStatus_SymbTab, SIZEOFARRAY(VetisVetDocStatus_SymbTab), temp_buf);
		else if(SXml::GetContentByName(p_a, "cargoInspected", temp_buf)) {
			SETFLAG(rResult.Flags, rResult.fCargoInspected, temp_buf.IsEqiAscii("true"));
		}
		else if(SXml::GetContentByName(p_a, "cargoExpertized", temp_buf)) {
			SETFLAG(rResult.Flags, rResult.fCargoExpertized, temp_buf.IsEqiAscii("true"));
		}
		else if(SXml::GetContentByName(p_a, "lastUpdateDate", temp_buf))
			rResult.LastUpdateDate = strtodate_(temp_buf, DATF_ISO8601);
		else if(SXml::GetContentByName(p_a, "waybillNumber", temp_buf))
			rResult.WayBillNumber = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		else if(SXml::GetContentByName(p_a, "waybillDate", temp_buf))
			rResult.WayBillDate = strtodate_(temp_buf, DATF_ISO8601);
		else if(SXml::GetContentByName(p_a, "waybillType", temp_buf))
			rResult.WayBillType = temp_buf.ToLong();
		else if(SXml::IsName(p_a, "locationProsperity")) {
		}
	}
	ParseCertifiedConsignment(pParentNode, r_crtc);
	{
		BarcodeTbl::Rec bc_rec;
		if(r_crtc.Batch.ProductMarkingList.getCount()) {
			PPIDArray link_goods_cadidate_list;
			for(uint ssp = 0; r_crtc.Batch.ProductMarkingList.get(&ssp, temp_buf);) {
				if(GObj.SearchByBarcode(temp_buf, &bc_rec, 0, 1) > 0) {
					link_goods_cadidate_list.add(bc_rec.GoodsID);
				}
			}
			link_goods_cadidate_list.sortAndUndup();
			if(link_goods_cadidate_list.getCount() == 1) {
				r_crtc.Batch.NativeGoodsID = link_goods_cadidate_list.get(0);
			}
		}
		if(!r_crtc.Batch.NativeGoodsID) {
			if(!!r_crtc.Batch.ProductItem.Guid) {
				VetisEntityCore::Entity ent_pi;
				if(PeC.GetEntityByGuid(r_crtc.Batch.ProductItem.Guid, ent_pi) > 0) {
					VetisProductItem pi;
					if(PeC.Get(ent_pi.ID, pi) > 0) {
						Goods2Tbl::Rec goods_rec;
						if(pi.NativeGoodsID > 0 && GObj.Fetch(pi.NativeGoodsID, &goods_rec) > 0) {
							r_crtc.Batch.NativeGoodsID = goods_rec.ID;
						}
						else if(pi.GlobalID.NotEmpty()) {
							if(GObj.SearchByBarcode(pi.GlobalID, &bc_rec, 0, 1) > 0) {
								r_crtc.Batch.NativeGoodsID = bc_rec.GoodsID;
							}
						}
					}
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

void SLAPI PPVetisInterface::ParseListResult(const xmlNode * pNode, VetisApplicationBlock::ReplyListValues & rResult)
{
	SString temp_buf;
	rResult.Count = SXml::GetAttrib(pNode, "count", temp_buf) ? temp_buf.ToLong() : 0;
	rResult.Offset = SXml::GetAttrib(pNode, "offset", temp_buf) ? temp_buf.ToLong() : 0;
	rResult.Total = SXml::GetAttrib(pNode, "total", temp_buf) ? temp_buf.ToLong() : 0;
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
					if(SXml::IsName(p_b, "receiveApplicationResultResponse") || SXml::IsName(p_b, "submitApplicationResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "application")) {
								for(xmlNode * p_a = p_i ? p_i->children : 0; p_a; p_a = p_a->next) {
									if(SXml::GetContentByName(p_a, "applicationId", temp_buf))
										rResult.ApplicationId.FromStr(temp_buf);
									else if(SXml::GetContentByName(p_a, "status", temp_buf))
										rResult.ApplicationStatus = SIntToSymbTab_GetId(VetisAppStatus_SymbTab, SIZEOFARRAY(VetisAppStatus_SymbTab), temp_buf);
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
											if(SXml::IsName(p_r, "getVetDocumentListResponse") || SXml::IsName(p_r, "getVetDocumentChangesListResponse")) {
												for(xmlNode * p_l = p_r->children; p_l; p_l = p_l->next) {
													if(SXml::IsName(p_l, "vetDocumentList")) {
														ParseListResult(p_l, rResult.ListResult);
														for(xmlNode * p_doci = p_l->children; p_doci; p_doci = p_doci->next) {
															if(SXml::IsName(p_doci, "vetDocument")) {
																VetisVetDocument * p_new_item = rResult.VetDocList.CreateNewItem();
																THROW_SL(p_new_item);
																THROW(ParseVetDocument(p_doci, *p_new_item));
															}
														}
													}
												}
											}
											else if(SXml::IsName(p_r, "getVetDocumentByUuidResponse")) {
												for(xmlNode * p_doci = p_r->children; p_doci; p_doci = p_doci->next) {
													if(SXml::IsName(p_doci, "vetDocument")) {
														VetisVetDocument * p_new_item = rResult.VetDocList.CreateNewItem();
														THROW_SL(p_new_item);
														THROW(ParseVetDocument(p_doci, *p_new_item));
													}
												}
											}
										}
									}
									else if(SXml::IsName(p_a, "errors")) {
										for(xmlNode * p_nerr = p_a->children; p_nerr; p_nerr = p_nerr->next) {
											if(SXml::GetContentByName(p_nerr, "error", temp_buf)) {
												VetisErrorEntry * p_err = rResult.ErrList.CreateNewItem();
												THROW_SL(p_err);
												p_err->Item = temp_buf.Strip().Transf(CTRANSF_UTF8_TO_INNER);
												if(SXml::GetAttrib(p_nerr, "code", temp_buf))
													p_err->Code = temp_buf.Strip().Transf(CTRANSF_UTF8_TO_INNER);
											}
										}
									}
								}
							}
						}
					}
					else if(SXml::IsName(p_b, "getProductItemListResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "productItemList")) {
								ParseListResult(p_i, rResult.ListResult);
								for(xmlNode * p_pr = p_i->children; p_pr; p_pr = p_pr->next) {
									if(SXml::IsName(p_pr, "productItem")) {
										VetisProductItem * p_new_item = rResult.ProductItemList.CreateNewItem();
										THROW(p_new_item);
										THROW(ParseProductItem(p_pr, *p_new_item));
									}
								}
							}
						}
					}
					else if(SXml::IsName(p_b, "getProductItemByGuidResponse") || SXml::IsName(p_b, "getProductItemByUuidResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "productItem")) {
								VetisProductItem * p_new_item = rResult.ProductItemList.CreateNewItem();
								THROW(p_new_item);
								THROW(ParseProductItem(p_i, *p_new_item));
							}
						}
					}
					else if(SXml::IsName(p_b, "getRussianEnterpriseListResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "enterpriseList")) {
								ParseListResult(p_i, rResult.ListResult);
								for(xmlNode * p_pr = p_i->children; p_pr; p_pr = p_pr->next) {
									if(SXml::IsName(p_pr, "enterprise")) {
										VetisEnterprise * p_new_item = rResult.EntItemList.CreateNewItem();
										THROW(p_new_item);
										THROW(ParseEnterprise(p_pr, *p_new_item));
									}
								}
							}
						}
					}
					else if(SXml::IsName(p_b, "getEnterpriseByGuidResponse") || SXml::IsName(p_b, "getEnterpriseByUuidResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "enterprise")) {
								VetisEnterprise * p_new_item = rResult.EntItemList.CreateNewItem();
								THROW(p_new_item);
								THROW(ParseEnterprise(p_i, *p_new_item));
							}
						}
					}
					else if(SXml::IsName(p_b, "getBusinessEntityListResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "businessEntityList")) {
								ParseListResult(p_i, rResult.ListResult);
								for(xmlNode * p_pr = p_i->children; p_pr; p_pr = p_pr->next) {
									if(SXml::IsName(p_pr, "businessEntity")) {
										VetisBusinessEntity * p_new_item = rResult.BEntList.CreateNewItem();
										THROW(p_new_item);
										THROW(ParseBusinessEntity(p_pr, *p_new_item));
									}
								}
							}
						}
					}
					else if(SXml::IsName(p_b, "getBusinessEntityByGuidResponse") || SXml::IsName(p_b, "getBusinessEntityByUuidResponse")) {
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::IsName(p_i, "businessEntity")) {
								VetisBusinessEntity * p_new_item = rResult.BEntList.CreateNewItem();
								THROW(p_new_item);
								THROW(ParseBusinessEntity(p_i, *p_new_item));
							}
						}
					}
					else if(SXml::IsName(p_b, "Fault")) {
						VetisFault single_fault;
						for(xmlNode * p_i = p_b->children; p_i; p_i = p_i->next) {
							if(SXml::GetContentByName(p_i, "faultcode", temp_buf)) {
							}
							else if(SXml::GetContentByName(p_i, "faultstring", temp_buf)) {
								single_fault.Message = temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
							}
							else if(SXml::GetContentByName(p_i, "faultactor", temp_buf)) {
							}
							else if(SXml::GetContentByName(p_i, "detail", temp_buf)) {
								static const SIntToSymbTabEntry fault_tab[] = {
									{ VetisFault::tUndef, "fault" },
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
						if(!single_fault.IsEmpty()) {
							VetisFault * p_fault = rResult.FaultList.CreateNewItem();
							THROW_SL(p_fault);
							*p_fault = single_fault;
						}
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
		/*
			error: MERC14082 Дата выработки продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14038 Вид транспорта в сведениях о принимаемой партии обязателен для заполнения.
			error: MERC14083 Дата окончания срока годности продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14094 Единица измерения в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14219 Страна происхождения продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14081 Скоропортящаяся продукция или нет в сведениях о принимаемой партии обязательно для заполнения.
		*/
		/*
			error: MERC14082 Дата выработки продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14083 Дата окончания срока годности продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14094 Единица измерения в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14040 Номер транспортного средства в сведениях о принимаемой партии обязателен для заполнения.
			error: MERC14219 Страна происхождения продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14081 Скоропортящаяся продукция или нет в сведениях о принимаемой партии обязательно для заполнения.
		*/
		/*
			error: MERC14082 Дата выработки продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14083 Дата окончания срока годности продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14219 Страна происхождения продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14081 Скоропортящаяся продукция или нет в сведениях о принимаемой партии обязательно для заполнения.
		*/
		/*
			error: MERC14082 Дата выработки продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14083 Дата окончания срока годности продукции в сведениях о принимаемой партии обязательна для заполнения.
			error: MERC14219 Страна происхождения продукции в сведениях о принимаемой партии обязательна для заполнения.
		*/
		/*
			error: MERC14219 Страна происхождения продукции в сведениях о принимаемой партии обязательна для заполнения.
		*/
		/*
			error: MERC14237 Список производителей в сведениях о принимаемой партии должен совпадать с указанным в ветеринарно-сопроводительном документе.
			error: MERC14260 Количество упаковки в запросе отличается от указанного в ветеринарно-сопроводительном документе более чем на 10% без указания причины в акте несоответствия.
		*/
		/*
			error: MERC14284 При указании количества единиц упаковки в сведениях о принимаемой партии необходимо также указать вид упаковки.
		*/
		/*
			error: MERC14237 Список производителей в сведениях о принимаемой партии должен совпадать с указанным в ветеринарно-сопроводительном документе.
		*/
		VetisSubmitRequestBlock srb;
		if(rAppBlk.P_AppParam->Sign == VetisApplicationData::signProcessIncomingConsignment) {
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			//xmlns="http://schemas.xmlsoap.org/soap/envelope/" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">  
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), InetUrl::MkHttp("schemas.xmlsoap.org", "soap/envelope/"));
			n_env.PutAttrib(SXml::nst("xmlns", "xs"),      InetUrl::MkHttp("www.w3.org", "2001/XMLSchema"));
			n_env.PutAttrib(SXml::nst("xmlns", "xsi"),     InetUrl::MkHttp("www.w3.org", "2001/XMLSchema-instance"));
			//n_env.PutAttrib(SXml::nst("xmlns", "ws"),      InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application/ws-definitions"));
			//n_env.PutAttrib(SXml::nst("xmlns", "app"),     InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application"));
			//n_env.PutAttrib(SXml::nst("xmlns", "v2"),      InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/registry/ws-definitions"));
			//n_env.PutAttrib(SXml::nst("xmlns", "v21"),     InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/dictionary"));
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst(/*"ws"*/0, "submitApplicationRequest"));
					//<submitApplicationRequest xmlns="http://api.vetrf.ru/schema/cdm/application/ws-definitions">
					n_f.PutAttrib("xmlns", InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application/ws-definitions"));
					P.GetExtStrData(extssApiKey, temp_buf);
					n_f.PutInner(SXml::nst(/*"ws"*/0, "apiKey"), temp_buf);
					{
						SXml::WNode n_app(srb, SXml::nst(/*"app"*/0, "application"));
						n_f.PutAttrib("xmlns", InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application"));
						n_app.PutInner(SXml::nst(/*"app"*/0, "applicationId"), rAppBlk.ApplicationId.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf));
						n_app.PutInner(SXml::nst(/*"app"*/0, "serviceId"), "mercury-g2b.service");
						n_app.PutInner(SXml::nst(/*"app"*/0, "issuerId"), rAppBlk.IssuerId.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf));
						n_app.PutInner(SXml::nst(/*"app"*/0, "issueDate"), temp_buf.Z().Cat(rAppBlk.IssueDate, DATF_ISO8601|DATF_CENTURY, /*TIMF_TIMEZONE*/0));
						SXml::WNode n_data(srb, SXml::nst(/*"app"*/0, "data"));
						{
							VetisProcessIncomingConsignmentRequest * p_req = (VetisProcessIncomingConsignmentRequest *)rAppBlk.P_AppParam;
							const VetisVetDocument & r_doc = p_req->Doc;
							const VetisBatch & r_bat = r_doc.CertifiedConsignment.Batch;
							SXml::WNode n_req(srb, "processIncomingConsignmentRequest");
							n_req.PutAttrib("xmlns",  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/applications"));
							//n_req.PutAttrib(SXml::nst("xmlns", "sch"), InetUrl::MkHttp("www.w3.org", "2001/XMLSchema"));
							//n_req.PutAttrib(SXml::nst("xmlns", "vd"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document"));
							n_req.PutAttrib(SXml::nst("xmlns", "bs"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
							//n_req.PutAttrib(SXml::nst("xmlns", "ent"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
							//n_req.PutAttrib(SXml::nst("xmlns", "co"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
							//n_req.PutAttrib(SXml::nst("xmlns", "pr"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
							//n_req.PutAttrib(SXml::nst("xmlns", "shp"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/shipment"));
							//n_env.PutAttrib(SXml::nst("xmlns", "dt"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/dictionary"));
							//n_req.PutAttrib(SXml::nst("xmlns", "ik"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar")); 
							n_req.PutInner("localTransactionId", temp_buf.Z().Cat(rAppBlk.LocalTransactionId));
							{
								SXml::WNode n_n2(srb, "initiator");
								n_n2.PutAttrib(SXml::nst("xmlns", "d7p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
								n_n2.PutInner(SXml::nst("d7p1", "login"), rAppBlk.User);
							}
							{
								SXml::WNode n_n2(srb, "delivery");
								n_n2.PutAttrib(SXml::nst("xmlns", "d7p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document"));
								LDATETIME dtm;
								dtm.Set(r_doc.IssueDate, ZEROTIME);
								n_n2.PutInner(SXml::nst("d7p1", "deliveryDate"), temp_buf.Z().Cat(dtm, DATF_ISO8601|DATF_CENTURY, 0));
								{
									SXml::WNode n_c(srb, SXml::nst("d7p1", "consignor"));
									//<d7p1:consignor xmlns:d8p1="http://api.vetrf.ru/schema/cdm/cerberus/enterprise">
									n_c.PutAttrib(SXml::nst("xmlns", "d8p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
									{
										SXml::WNode n_e(srb, SXml::nst("d8p1", "businessEntity"));
										n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignor.BusinessEntity.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
									{
										SXml::WNode n_e(srb, SXml::nst("d8p1", "enterprise"));
										n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignor.Enterprise.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
								}
								{
									SXml::WNode n_c(srb, SXml::nst("d7p1", "consignee"));
									n_c.PutAttrib(SXml::nst("xmlns", "d8p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
									{
										SXml::WNode n_e(srb, SXml::nst("d8p1", "businessEntity"));
										n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignee.BusinessEntity.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
									{
										SXml::WNode n_e(srb, SXml::nst("d8p1", "enterprise"));
										n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignee.Enterprise.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
								}
								{
									SXml::WNode n_c(srb, SXml::nst("d7p1", "consignment"));
									n_c.PutInner(SXml::nst("d7p1", "productType"), temp_buf.Z().Cat(r_bat.ProductType));
									{
										SXml::WNode n_p(srb, SXml::nst("d7p1", "product"));
										n_p.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
										n_p.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_bat.Product.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
									{
										SXml::WNode n_p(srb, SXml::nst("d7p1", "subProduct"));
										n_p.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
										n_p.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_bat.SubProduct.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
									{
										SXml::WNode n_p(srb, SXml::nst("d7p1", "productItem"));
										n_p.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
										if(!r_bat.ProductItem.Guid.IsZero())
											n_p.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_bat.ProductItem.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
										temp_buf = r_bat.ProductItem.Name;
										if(temp_buf.NotEmptyS()) {
											n_p.PutInner(SXml::nst("d9p1", "name"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
										}
									}
									n_c.PutInner(SXml::nst("d7p1", "volume"), temp_buf.Z().Cat(r_bat.Volume, MKSFMTD(0, 6, NMBF_NOTRAILZ)));
									{
										SXml::WNode n_uom(srb, SXml::nst("d7p1", "unit"));
										n_uom.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
										if(!r_bat.Unit.Uuid.IsZero())
											n_uom.PutInner(SXml::nst("bs", "uuid"), temp_buf.Z().Cat(r_bat.Unit.Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
										if(!r_bat.Unit.Guid.IsZero())
											n_uom.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_bat.Unit.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
									if(r_bat.PackingAmount > 0) {
										if(r_bat.PackingList.getCount()) {
											SXml::WNode n_pl(srb, SXml::nst("d7p1", "packingList"));
											n_pl.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
											for(uint pli = 0; pli < r_bat.PackingList.getCount(); pli++) {
												const VetisNamedGenericVersioningEntity * p_pl_item = r_bat.PackingList.at(pli);
												if(p_pl_item) {
													SXml::WNode n_pf(srb, SXml::nst("d9p1", "packingForm"));
													if(!p_pl_item->Uuid.IsZero())
														n_pf.PutInner(SXml::nst("bs", "uuid"), temp_buf.Z().Cat(p_pl_item->Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
													if(!p_pl_item->Guid.IsZero())
														n_pf.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(p_pl_item->Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
													if(p_pl_item->Name.NotEmpty())
														n_pf.PutInner(SXml::nst("d9p1", "name"), (temp_buf = p_pl_item->Name).Transf(CTRANSF_INNER_TO_UTF8));
												}
											}
										}
										n_c.PutInner(SXml::nst("d7p1", "packingAmount"), temp_buf.Z().Cat(r_bat.PackingAmount));
									}
									{
										SXml::WNode n_dt(srb, SXml::nst("d7p1", "dateOfProduction"));
										PutGoodsDate(srb, SXml::nst("d7p1", "firstDate"), r_bat.DateOfProduction.FirstDate);
										PutGoodsDate(srb, SXml::nst("d7p1", "secondDate"), r_bat.DateOfProduction.SecondDate);
									}
									{
										SXml::WNode n_dt(srb, SXml::nst("d7p1", "expiryDate"));
										PutGoodsDate(srb, SXml::nst("d7p1", "firstDate"), r_bat.ExpiryDate.FirstDate);
										PutGoodsDate(srb, SXml::nst("d7p1", "secondDate"), r_bat.ExpiryDate.SecondDate);
									}
									n_c.PutInner(SXml::nst("d7p1", "perishable"), temp_buf.Z().Cat((r_bat.Flags & VetisBatch::fPerishable) ? "true" : "false"));
									{
										SXml::WNode n_country(srb, SXml::nst("d7p1", "countryOfOrigin"));
										n_country.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar"));
										if(!r_bat.Origin.Country.Uuid.IsZero())
											n_country.PutInner(SXml::nst("bs", "uuid"), temp_buf.Z().Cat(r_bat.Origin.Country.Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
										if(!r_bat.Origin.Country.Guid.IsZero())
											n_country.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_bat.Origin.Country.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
									if(r_bat.Origin.Producer.getCount()) {
										SXml::WNode n_pl(srb, SXml::nst("d7p1", "producerList"));
										n_pl.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
										for(uint pli = 0; pli < r_bat.Origin.Producer.getCount(); pli++) {
											const VetisProducer * p_pl_item = r_bat.Origin.Producer.at(pli);
											if(p_pl_item) {
												SXml::WNode n_producer(srb, SXml::nst("d9p1", "producer"));
												{
													SXml::WNode n_ent(srb, SXml::nst("d9p1", "enterprise"));
													if(!p_pl_item->Uuid.IsZero())
														n_ent.PutInner(SXml::nst("bs", "uuid"), temp_buf.Z().Cat(p_pl_item->Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
													if(!p_pl_item->Guid.IsZero())
														n_ent.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(p_pl_item->Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
													if(p_pl_item->Name.NotEmpty())
														n_ent.PutInner(SXml::nst("d9p1", "name"), (temp_buf = p_pl_item->Name).Transf(CTRANSF_INNER_TO_UTF8));
												}
												const char * p_role = "UNKNOWN";
												if(p_pl_item->Role == p_pl_item->rolePRODUCER)
													p_role = "PRODUCER";
												n_producer.PutInner(SXml::nst("d9p1", "role"), p_role);
											}
										}
									}
								}
								{
									//SXml::WNode n_c(srb, SXml::nst("vd", "transportInfo"));
								}
								{
									SXml::WNode n_af(srb, SXml::nst("d7p1", "accompanyingForms"));
									{
										SXml::WNode n_wb(srb, SXml::nst("d7p1", "waybill"));
										n_wb.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/shipment"));
										n_wb.PutInner(SXml::nst("d9p1", "issueNumber"), (temp_buf = r_doc.WayBillNumber).Transf(CTRANSF_INNER_TO_UTF8));
										n_wb.PutInner(SXml::nst("d9p1", "issueDate"), temp_buf.Z().Cat(r_doc.WayBillDate, DATF_ISO8601|DATF_CENTURY));
										n_wb.PutInner(SXml::nst("d9p1", "type"), "1");
										{
											SXml::WNode n_tr(srb, SXml::nst("d9p1", "transportInfo"));
											if(r_doc.CertifiedConsignment.TransportInfo.TransportType) {
												temp_buf.Z().Cat(r_doc.CertifiedConsignment.TransportInfo.TransportType);
												n_tr.PutInner(SXml::nst("d9p1", "transportType"), temp_buf);
											}
											if(!r_doc.CertifiedConsignment.TransportInfo.TransportNumber.IsEmpty()) {
												SXml::WNode n_tn(srb, SXml::nst("d9p1", "transportNumber"));
												temp_buf = r_doc.CertifiedConsignment.TransportInfo.TransportNumber.VehicleNumber;
												if(temp_buf.NotEmptyS())
													n_tn.PutInner(SXml::nst("d9p1", "vehicleNumber"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												temp_buf = r_doc.CertifiedConsignment.TransportInfo.TransportNumber.TrailerNumber;
												if(temp_buf.NotEmptyS())
													n_tn.PutInner(SXml::nst("d9p1", "trailerNumber"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												temp_buf = r_doc.CertifiedConsignment.TransportInfo.TransportNumber.ContainerNumber;
												if(temp_buf.NotEmptyS())
													n_tn.PutInner(SXml::nst("d9p1", "containerNumber"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												temp_buf = r_doc.CertifiedConsignment.TransportInfo.TransportNumber.WagonNumber;
												if(temp_buf.NotEmptyS())
													n_tn.PutInner(SXml::nst("d9p1", "wagonNumber"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												temp_buf = r_doc.CertifiedConsignment.TransportInfo.TransportNumber.ShipName;
												if(temp_buf.NotEmptyS())
													n_tn.PutInner(SXml::nst("d9p1", "shipName"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												temp_buf = r_doc.CertifiedConsignment.TransportInfo.TransportNumber.FlightNumber;
												if(temp_buf.NotEmptyS())
													n_tn.PutInner(SXml::nst("d9p1", "flightNumber"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
											}
										}
										if(r_doc.CertifiedConsignment.TransportStorageType) {
											if(SIntToSymbTab_GetSymb(VetisTranspStorageType_SymbTab, SIZEOFARRAY(VetisTranspStorageType_SymbTab), 
												r_doc.CertifiedConsignment.TransportStorageType, temp_buf)) {
												n_n2.PutInner(SXml::nst("d9p1", "transportStorageType"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
											}
										}
									}
									{
										SXml::WNode n_vc(srb, SXml::nst("d7p1", "vetCertificate"));
										n_vc.PutInner(SXml::nst("bs", "uuid"), temp_buf.Z().Cat(r_doc.Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
										/*
										temp_buf = r_doc.IssueSeries;
										if(temp_buf.NotEmptyS())
											n_vc.PutInner(SXml::nst("vd", "issueSeries"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
										temp_buf = r_doc.IssueNumber;
										if(temp_buf.NotEmptyS())
											n_vc.PutInner(SXml::nst("vd", "issueNumber"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
										if(checkdate(r_doc.IssueDate))
											n_vc.PutInner(SXml::nst("vd", "issueDate"), temp_buf.Z().Cat(r_doc.IssueDate, DATF_ISO8601|DATF_CENTURY));
										n_vc.PutInner(SXml::nst("vd", "form"), "");
										*/
									}
								}
							}
							{
								SXml::WNode n_n2(srb, "deliveryFacts");
								//<deliveryFacts xmlns:d7p1="http://api.vetrf.ru/schema/cdm/mercury/vet-document">  
								n_n2.PutAttrib(SXml::nst("xmlns", "d7p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document"));
								n_n2.PutInner(SXml::nst("d7p1", "vetCertificatePresence"), "ELECTRONIC");
								{
									SXml::WNode n_c(srb, SXml::nst("d7p1", "docInspection"));
									{
										SXml::WNode n_i(srb, SXml::nst("d7p1", "responsible"));
										//<d7p1:responsible xmlns:d9p1="http://api.vetrf.ru/schema/cdm/argus/common">
										n_i.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
										n_i.PutInner(SXml::nst("d9p1", "login"), rAppBlk.User);
									}
									n_c.PutInner(SXml::nst("d7p1", "result"), "CORRESPONDS");
								}
								{
									SXml::WNode n_c(srb, SXml::nst("d7p1", "vetInspection"));
									{
										SXml::WNode n_i(srb, SXml::nst("d7p1", "responsible"));
										n_i.PutAttrib(SXml::nst("xmlns", "d9p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
										n_i.PutInner(SXml::nst("d9p1", "login"), rAppBlk.User);
									}
									n_c.PutInner(SXml::nst("d7p1", "result"), "CORRESPONDS");
								}
								n_n2.PutInner(SXml::nst("d7p1", "decision"), "ACCEPT_ALL");
							}
							/*
							{
								SXml::WNode n_n2(srb, "discrepancyReport");
								//<discrepancyReport xmlns:d7p1="http://api.vetrf.ru/schema/cdm/mercury/vet-document">
								n_n2.PutAttrib(SXml::nst("xmlns", "d7p1"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document"));
								n_n2.PutInner(SXml::nst("d7p1", "issueDate"), temp_buf.Z().Cat(getcurdate_(), DATF_ISO8601|DATF_CENTURY));
								{
									SXml::WNode n_reason(srb, SXml::nst("d7p1", "reason"), temp_buf = "abc");
								}
								n_n2.PutInner(SXml::nst("d7p1", "description"), temp_buf = "abc");
							}
							*/
						}
					}
				}
			}
		}
		else 
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), InetUrl::MkHttp("schemas.xmlsoap.org", "soap/envelope/"));
			n_env.PutAttrib(SXml::nst("xmlns", "ws"),      InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application/ws-definitions"));
			n_env.PutAttrib(SXml::nst("xmlns", "app"),     InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application"));
			n_env.PutAttrib(SXml::nst("xmlns", "v2"),      InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/registry/ws-definitions/v2"));
			n_env.PutAttrib(SXml::nst("xmlns", "v21"),     InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/dictionary/v2"));
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
						n_app.PutInner(SXml::nst("app", "serviceId"), (rAppBlk.VetisSvcVer == 2) ? "mercury-g2b.service:2.0" : "mercury-g2b.service");
						n_app.PutInner(SXml::nst("app", "issuerId"), rAppBlk.IssuerId.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf));
						n_app.PutInner(SXml::nst("app", "issueDate"), temp_buf.Z().Cat(rAppBlk.IssueDate, DATF_ISO8601|DATF_CENTURY, /*TIMF_TIMEZONE*/0));
						SXml::WNode n_data(srb, SXml::nst("app", "data"));
						switch(rAppBlk.P_AppParam->Sign) {
							case VetisApplicationData::signProcessIncomingConsignment:
								{
									VetisProcessIncomingConsignmentRequest * p_req = (VetisProcessIncomingConsignmentRequest *)rAppBlk.P_AppParam;
									const VetisVetDocument & r_doc = p_req->Doc;
									SXml::WNode n_req(srb, "processIncomingConsignmentRequest");
									n_req.PutAttrib(SXml::nst("xmlns", "sch"), InetUrl::MkHttp("www.w3.org", "2001/XMLSchema"));
									n_req.PutAttrib(SXml::nst("xmlns", "vd"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document/v2"));
									n_req.PutAttrib(SXml::nst("xmlns", "bs"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
									n_req.PutAttrib(SXml::nst("xmlns", "ent"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
									n_req.PutAttrib(SXml::nst("xmlns", "co"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
									n_req.PutAttrib(SXml::nst("xmlns", "pr"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
									n_req.PutAttrib(SXml::nst("xmlns", "shp"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/shipment"));
									n_env.PutAttrib(SXml::nst("xmlns", "dt"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/dictionary/v2"));
									n_req.PutAttrib(SXml::nst("xmlns", "ik"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar")); 
									n_req.PutAttrib("xmlns", InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/applications/v2"));
									n_req.PutInner("localTransactionId", temp_buf.Z().Cat(rAppBlk.LocalTransactionId));
									{
										SXml::WNode n_n2(srb, "initiator");
										n_n2.PutInner(SXml::nst("co", "login"), rAppBlk.User);
									}
									{
										SXml::WNode n_n2(srb, "delivery");
										LDATETIME dtm;
										dtm.Set(r_doc.IssueDate, ZEROTIME);
										n_n2.PutInner(SXml::nst("vd", "deliveryDate"), temp_buf.Z().Cat(dtm, DATF_ISO8601|DATF_CENTURY, 0));
										{
											SXml::WNode n_c(srb, SXml::nst("vd", "consignor"));
											{
												SXml::WNode n_e(srb, SXml::nst("ent", "businessEntity"));
												n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignor.BusinessEntity.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
											}
											{
												SXml::WNode n_e(srb, SXml::nst("ent", "enterprise"));
												n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignor.Enterprise.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
											}
										}
										{
											SXml::WNode n_c(srb, SXml::nst("vd", "consignee"));
											{
												SXml::WNode n_e(srb, SXml::nst("ent", "businessEntity"));
												n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignee.BusinessEntity.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
											}
											{
												SXml::WNode n_e(srb, SXml::nst("ent", "enterprise"));
												n_e.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Consignee.Enterprise.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
											}
										}
										{
											SXml::WNode n_c(srb, SXml::nst("vd", "consignment"));
											n_c.PutInner(SXml::nst("vd", "productType"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Batch.ProductType));
											{
												SXml::WNode n_p(srb, SXml::nst("vd", "product"));
												n_p.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Batch.Product.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
											}
											{
												SXml::WNode n_p(srb, SXml::nst("vd", "subProduct"));
												n_p.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Batch.SubProduct.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
											}
											{
												SXml::WNode n_p(srb, SXml::nst("vd", "productItem"));
												if(!r_doc.CertifiedConsignment.Batch.ProductItem.Guid.IsZero())
													n_p.PutInner(SXml::nst("bs", "guid"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Batch.ProductItem.Guid, S_GUID::fmtIDL|S_GUID::fmtLower));
												temp_buf = r_doc.CertifiedConsignment.Batch.ProductItem.Name;
												if(temp_buf.NotEmptyS()) {
													n_p.PutInner(SXml::nst("pr", "name"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												}
											}
											n_c.PutInner(SXml::nst("vd", "volume"), temp_buf.Z().Cat(r_doc.CertifiedConsignment.Batch.Volume, MKSFMTD(0, 6, NMBF_NOTRAILZ)));
										}
										{
											//SXml::WNode n_c(srb, SXml::nst("vd", "transportInfo"));
										}
										{
											SXml::WNode n_af(srb, SXml::nst("vd", "accompanyingForms"));
											{
												SXml::WNode n_vc(srb, SXml::nst("vd", "vetCertificate"));
												n_vc.PutInner(SXml::nst("bs", "uuid"), temp_buf.Z().Cat(r_doc.Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
												/*
												temp_buf = r_doc.IssueSeries;
												if(temp_buf.NotEmptyS())
													n_vc.PutInner(SXml::nst("vd", "issueSeries"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												temp_buf = r_doc.IssueNumber;
												if(temp_buf.NotEmptyS())
													n_vc.PutInner(SXml::nst("vd", "issueNumber"), temp_buf.Transf(CTRANSF_INNER_TO_UTF8));
												if(checkdate(r_doc.IssueDate))
													n_vc.PutInner(SXml::nst("vd", "issueDate"), temp_buf.Z().Cat(r_doc.IssueDate, DATF_ISO8601|DATF_CENTURY));
												n_vc.PutInner(SXml::nst("vd", "form"), "");
												*/
											}
										}
										//n_n2.PutInner(SXml::nst("vd", "transportStorageType"), "VENTILATED");
									}
									{
										SXml::WNode n_n2(srb, "deliveryFacts");
										n_n2.PutInner(SXml::nst("vd", "vetCertificatePresence"), "ELECTRONIC");
										{
											SXml::WNode n_c(srb, SXml::nst("vd", "docInspection"));
											{
												SXml::WNode n_i(srb, SXml::nst("vd", "responsible"));
												n_i.PutInner(SXml::nst("co", "login"), rAppBlk.User);
											}
											n_c.PutInner(SXml::nst("vd", "result"), "CORRESPONDS");
										}
										{
											SXml::WNode n_c(srb, SXml::nst("vd", "vetInspection"));
											{
												SXml::WNode n_i(srb, SXml::nst("vd", "responsible"));
												n_i.PutInner(SXml::nst("co", "login"), rAppBlk.User);
											}
											n_c.PutInner(SXml::nst("vd", "result"), "CORRESPONDS");
										}
										n_n2.PutInner(SXml::nst("vd", "decision"), "ACCEPT_ALL");
									}
									{
										SXml::WNode n_n2(srb, "discrepancyReport");
										/*<v21:reason>  
										<v21:name>nonono</v21:name>  
										</v21:reason>  
										<v21:description>lalala</v21:description>*/
										{
											SXml::WNode n_reason(srb, SXml::nst("vd", "reason"), temp_buf = "Идите на хуй со своим меркурием!");
										}
										n_n2.PutInner(SXml::nst("vd", "description"), temp_buf = "И снова идите на хуй со своим меркурием!");
									}
								}
								break;
							case VetisApplicationData::signGetVetDocumentByUuid:
								{
									VetisGetVetDocumentByUuidRequest * p_req = (VetisGetVetDocumentByUuidRequest *)rAppBlk.P_AppParam;
									SXml::WNode n_req(srb, "getVetDocumentByUuidRequest");
									n_req.PutAttrib(SXml::nst("xmlns", "sch"), InetUrl::MkHttp("www.w3.org", "2001/XMLSchema"));
									n_req.PutAttrib(SXml::nst("xmlns", "vd"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document"));
									n_req.PutAttrib(SXml::nst("xmlns", "sh"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/shipment"));
									n_req.PutAttrib(SXml::nst("xmlns", "ws"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application/ws-definitions"));
									n_req.PutAttrib(SXml::nst("xmlns", "app"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application"));
									n_req.PutAttrib(SXml::nst("xmlns", "co"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
									n_req.PutAttrib(SXml::nst("xmlns", "ent"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
									n_req.PutAttrib(SXml::nst("xmlns", "pr"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
									n_req.PutAttrib(SXml::nst("xmlns", "ik"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar")); 
									n_req.PutAttrib(SXml::nst("xmlns", "bs"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
									n_req.PutAttrib("xmlns", InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/applications"));
									n_req.PutInner("localTransactionId", temp_buf.Z().Cat(rAppBlk.LocalTransactionId));
									{
										SXml::WNode n_n2(srb, "initiator");
										n_n2.PutInner(SXml::nst("co", "login"), rAppBlk.User);
									}
									{
										SXml::WNode n_n2(srb, SXml::nst("bs", "uuid"), temp_buf.Z().Cat(p_req->Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
									}
									n_req.PutInner(SXml::nst("ent", "enterpriseGuid"), temp_buf.Z().Cat(rAppBlk.EnterpriseId, S_GUID::fmtIDL|S_GUID::fmtLower));
								}
								break;
							case VetisApplicationData::signGetVetDocumentChangesList:
								{
									VetisGetVetDocumentChangesListRequest * p_req = (VetisGetVetDocumentChangesListRequest *)rAppBlk.P_AppParam;
									SXml::WNode n_req(srb, "getVetDocumentChangesListRequest");
									n_req.PutAttrib(SXml::nst("xmlns", "sch"), InetUrl::MkHttp("www.w3.org",   "2001/XMLSchema"));
									n_req.PutAttrib(SXml::nst("xmlns", "vd"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document"));
									n_req.PutAttrib(SXml::nst("xmlns", "sh"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/shipment"));
									n_req.PutAttrib(SXml::nst("xmlns", "ws"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application/ws-definitions"));
									n_req.PutAttrib(SXml::nst("xmlns", "app"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application"));
									n_req.PutAttrib(SXml::nst("xmlns", "co"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
									n_req.PutAttrib(SXml::nst("xmlns", "ent"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
									n_req.PutAttrib(SXml::nst("xmlns", "pr"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
									n_req.PutAttrib(SXml::nst("xmlns", "ik"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar")); 
									n_req.PutAttrib(SXml::nst("xmlns", "bs"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
									n_req.PutAttrib("xmlns", InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/applications"));
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
									{
										SXml::WNode n_n2(srb, SXml::nst("bs", "updateDateInterval"));
										n_n2.PutInner(SXml::nst("bs", "beginDate"), temp_buf.Z().Cat(p_req->Period.Start, DATF_ISO8601|DATF_CENTURY, 0));
										n_n2.PutInner(SXml::nst("bs", "endDate"), temp_buf.Z().Cat(p_req->Period.Finish, DATF_ISO8601|DATF_CENTURY, 0));
									}
									n_req.PutInner(SXml::nst("ent", "enterpriseGuid"), temp_buf.Z().Cat(rAppBlk.EnterpriseId, S_GUID::fmtIDL|S_GUID::fmtLower));
								}
								break;
							case VetisApplicationData::signGetVetDocumentList:
								{ 
									VetisGetVetDocumentListRequest * p_req = (VetisGetVetDocumentListRequest *)rAppBlk.P_AppParam;
									SXml::WNode n_req(srb, "getVetDocumentListRequest");
									n_req.PutAttrib(SXml::nst("xmlns", "sch"), InetUrl::MkHttp("www.w3.org", "2001/XMLSchema"));
									n_req.PutAttrib(SXml::nst("xmlns", "vd"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/vet-document"));
									n_req.PutAttrib(SXml::nst("xmlns", "sh"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/shipment"));
									n_req.PutAttrib(SXml::nst("xmlns", "ws"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application/ws-definitions"));
									n_req.PutAttrib(SXml::nst("xmlns", "app"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application"));
									n_req.PutAttrib(SXml::nst("xmlns", "co"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/common"));
									n_req.PutAttrib(SXml::nst("xmlns", "ent"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
									n_req.PutAttrib(SXml::nst("xmlns", "pr"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/argus/production"));
									n_req.PutAttrib(SXml::nst("xmlns", "ik"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar")); 
									n_req.PutAttrib(SXml::nst("xmlns", "bs"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
									n_req.PutAttrib("xmlns", InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/mercury/applications"));
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
		LogFaults(rResult);
	}
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::LogFaults(const VetisApplicationBlock & rAb)
{
	int    ok = -1;
	if(rAb.FaultList.getCount() || rAb.ErrList.getCount()) {
		ok = 1;
		if(P_Logger) {
			SString temp_buf;
			for(uint fi = 0; fi < rAb.FaultList.getCount(); fi++) {
				const VetisFault * p_f = rAb.FaultList.at(fi);
				if(p_f) {
					temp_buf.Z().Cat("fault").CatDiv(':', 2).Cat(p_f->Message);
					P_Logger->Log(temp_buf);
					for(uint fei = 0; fei < p_f->ErrList.getCount(); fei++) {
						const VetisErrorEntry * p_e = p_f->ErrList.at(fei);
						if(p_e) {
							temp_buf.Z().Cat("error").CatDiv(':', 0);
							if(p_e->Code.NotEmpty())
								temp_buf.Space().Cat(p_e->Code);
							if(p_e->Item.NotEmpty())
								temp_buf.Space().Cat(p_e->Item);
							P_Logger->Log(temp_buf);
						}
					}
				}
			}
			for(uint ei = 0; ei < rAb.ErrList.getCount(); ei++) {
				const VetisErrorEntry * p_e = rAb.ErrList.at(ei);
				if(p_e) {
					temp_buf.Z().Cat("error").CatDiv(':', 0);
					if(p_e->Code.NotEmpty())
						temp_buf.Space().Cat(p_e->Code);
					if(p_e->Item.NotEmpty())
						temp_buf.Space().Cat(p_e->Item);
					P_Logger->Log(temp_buf);
				}
			}
		}
	}
	return ok;
}

int SLAPI PPVetisInterface::ReceiveResult(const S_GUID & rAppId, VetisApplicationBlock & rResult)
{
	int    ok = -1;
	uint   try_count = 0;
	SString temp_buf;
	SString reply_buf;
	THROW(State & stInited);
	do {
		rResult.Clear();
		{
			if(try_count == 0)
				SDelay(200);
			else
				SDelay(500);
			try_count++;
		}
		VetisSubmitRequestBlock srb;
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), InetUrl::MkHttp("schemas.xmlsoap.org", "soap/envelope/"));
			n_env.PutAttrib(SXml::nst("xmlns", "ws"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/application/ws-definitions"));
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
		if(rResult.ApplicationStatus == rResult.appstRejected)
			LogFaults(rResult);
	} while(rResult.ApplicationStatus == rResult.appstInProcess);
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetEntityQuery(int queryType, const char * pQueryParam, VetisApplicationBlock & rReply)
{
	int    ok = -1;
	SString temp_buf;
	SString reply_buf;
	THROW(State & stInited);
	THROW(!isempty(pQueryParam));
	{
		enum {
			svcUndef      = 0,
			svcProduct    = 1,
			svcEnterprise = 2
		};
		int    svc = svcUndef;
		const char * p_soap_req = 0;
		const char * p_soap_action = 0;
		SString param_tag;
		VetisSubmitRequestBlock srb;
		switch(queryType) {
			case qtProductItemByGuid: 
				svc = svcProduct;
				p_soap_req = "getProductItemByGuidRequest";
				p_soap_action = "GetProductItemByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtProductItemByUuid:
				svc = svcProduct;
				p_soap_req = "getProductItemByUuidRequest";
				p_soap_action = "GetProductItemByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
			case qtProductByGuid:
				svc = svcProduct;
				p_soap_req = "getProductByGuidRequest";
				p_soap_action = "GetProductByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtProductByUuid:
				svc = svcProduct;
				p_soap_req = "getProductByUuidRequest";
				p_soap_action = "GetProductByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
			case qtSubProductByGuid:
				svc = svcProduct;
				p_soap_req = "getSubProductByGuidRequest";
				p_soap_action = "GetSubProductByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtSubProductByUuid:
				svc = svcProduct;
				p_soap_req = "getSubProductByUuidRequest";
				p_soap_action = "GetSubProductByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
			case qtBusinessEntityByGuid:
				svc = svcEnterprise;
				p_soap_req = "getBusinessEntityByGuidRequest";
				p_soap_action = "GetBusinessEntityByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtBusinessEntityByUuid:
				svc = svcEnterprise;
				p_soap_req = "getBusinessEntityByUuidRequest";
				p_soap_action = "GetBusinessEntityByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
			case qtEnterpriseByGuid:
				svc = svcEnterprise;
				p_soap_req = "getEnterpriseByGuidRequest";
				p_soap_action = "GetEnterpriseByGuid";
				param_tag = SXml::nst("base", "guid");
				break;
			case qtEnterpriseByUuid:
				svc = svcEnterprise;
				p_soap_req = "getEnterpriseByUuidRequest";
				p_soap_action = "GetEnterpriseByUuid";
				param_tag = SXml::nst("base", "uuid");
				break;
		}
		if(oneof2(svc, svcProduct, svcEnterprise)) {
			const char * p_url = 0;
			const char * p_arg_ns = 0;
			{
				SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
				n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), InetUrl::MkHttp("schemas.xmlsoap.org", "soap/envelope/"));
				n_env.PutAttrib(SXml::nst("xmlns", "base"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
				if(svc == svcProduct) {
					p_url = InetUrl::MkHttps("api.vetrf.ru", "platform/services/2.0/ProductService");
					n_env.PutAttrib(SXml::nst("xmlns", "v2"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/registry/ws-definitions/v2"));
					n_env.PutAttrib(SXml::nst("xmlns", "v21"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/dictionary/v2"));
					p_arg_ns = "v2";
				}
				else if(svc == svcEnterprise) {
					p_url = InetUrl::MkHttps("api.vetrf.ru", "platform/cerberus/services/EnterpriseService");
					if(oneof2(queryType, qtBusinessEntityByGuid, qtBusinessEntityByUuid)) {
						n_env.PutAttrib(SXml::nst("xmlns", "ws"), "http://api.vetrf.ru/schema/cdm/cerberus/business-entity/ws-definitions");
					}
					else if(oneof2(queryType, qtEnterpriseByGuid, qtEnterpriseByUuid)) {
						n_env.PutAttrib(SXml::nst("xmlns", "ws"), "http://api.vetrf.ru/schema/cdm/cerberus/enterprise/ws-definitions");
					}
					p_arg_ns = "ws";
				}
				assert(p_url);
				{
					SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
				}
				{
					SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
					{
						SXml::WNode n_f(srb, SXml::nst(p_arg_ns, p_soap_req));
						n_f.PutInner(param_tag, pQueryParam);
					}
				}
			}
			xmlTextWriterFlush(srb);
			reply_buf.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
			THROW(SendSOAP(p_url, p_soap_action, reply_buf, temp_buf));
			THROW(ParseReply(temp_buf, rReply));
			ok = 1;
		}
	}
    CATCHZOK
	return ok;
}

void SLAPI PPVetisInterface::PutListOptionsParam(xmlTextWriter * pWriter, uint offs, uint count)
{
	SString temp_buf;
	SXml::WNode n_lo(pWriter, SXml::nst("base", "listOptions"));
	n_lo.PutInner(SXml::nst("base", "count"), temp_buf.Z().Cat(count));
	n_lo.PutInner(SXml::nst("base", "offset"), temp_buf.Z().Cat(offs));
}

int SLAPI PPVetisInterface::PutGoodsDate(xmlTextWriter * pWriter, const char * pScopeXmlTag, const SUniTime & rUt)
{
	int    ok = -1;
	const  uint uts = rUt.GetSignature();
	if(oneof4(uts, SUniTime::indYr, SUniTime::indMon, SUniTime::indDay, SUniTime::indHr)) {
		LDATETIME dtm;
		if(rUt.Get(dtm)) {
			SString temp_buf;
			SXml::WNode n_d2(pWriter, pScopeXmlTag);
			if(uts == SUniTime::indYr) {
				n_d2.PutInner(SXml::nst("bs", "year"), temp_buf.Z().Cat(dtm.d.year()));
			}
			else if(uts == SUniTime::indMon) {
				n_d2.PutInner(SXml::nst("bs", "year"), temp_buf.Z().Cat(dtm.d.year()));
				n_d2.PutInner(SXml::nst("bs", "month"), temp_buf.Z().Cat(dtm.d.month()));
			}
			else if(uts == SUniTime::indDay) {
				n_d2.PutInner(SXml::nst("bs", "year"), temp_buf.Z().Cat(dtm.d.year()));
				n_d2.PutInner(SXml::nst("bs", "month"), temp_buf.Z().Cat(dtm.d.month()));
				n_d2.PutInner(SXml::nst("bs", "day"), temp_buf.Z().Cat(dtm.d.day()));
			}
			else if(uts == SUniTime::indHr) {
				n_d2.PutInner(SXml::nst("bs", "year"), temp_buf.Z().Cat(dtm.d.year()));
				n_d2.PutInner(SXml::nst("bs", "month"), temp_buf.Z().Cat(dtm.d.month()));
				n_d2.PutInner(SXml::nst("bs", "day"), temp_buf.Z().Cat(dtm.d.day()));
				n_d2.PutInner(SXml::nst("bs", "hour"), temp_buf.Z().Cat(dtm.t.hour()));
			}
			ok = 1;
		}
		else
			ok = 0;
	}
	return ok;
}

int SLAPI PPVetisInterface::GetProductItemList(uint offs, uint count, VetisApplicationBlock & rReply)
{
	int    ok = -1;
	SString temp_buf;
	SString reply_buf;
	THROW(State & stInited);
	{
		VetisSubmitRequestBlock srb;
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), InetUrl::MkHttp("schemas.xmlsoap.org", "soap/envelope/"));
			n_env.PutAttrib(SXml::nst("xmlns", "v2"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/registry/ws-definitions/v2"));
			n_env.PutAttrib(SXml::nst("xmlns", "base"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
			n_env.PutAttrib(SXml::nst("xmlns", "v21"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/dictionary/v2"));
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst("v2", "getProductItemListRequest"));
					PutListOptionsParam(srb, offs, count);
				}
			}
		}
		xmlTextWriterFlush(srb);
		reply_buf.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
		THROW(SendSOAP(InetUrl::MkHttps("api.vetrf.ru", "platform/services/2.0/ProductService"), "GetProductItemList", reply_buf, temp_buf));
		THROW(ParseReply(temp_buf, rReply));
	}
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetRussianEnterpriseList(uint offs, uint count, VetisApplicationBlock & rReply)
{
	int    ok = -1;
	SString temp_buf;
	SString reply_buf;
	THROW(State & stInited);
	{
		VetisSubmitRequestBlock srb;
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), InetUrl::MkHttp("schemas.xmlsoap.org", "soap/envelope/"));
			n_env.PutAttrib(SXml::nst("xmlns", "ws"), "http://api.vetrf.ru/schema/cdm/cerberus/enterprise/ws-definitions");
			n_env.PutAttrib(SXml::nst("xmlns", "base"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
			n_env.PutAttrib(SXml::nst("xmlns", "v2"),   InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/registry/ws-definitions/v2"));
			n_env.PutAttrib(SXml::nst("xmlns", "ent"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
			n_env.PutAttrib(SXml::nst("xmlns", "v21"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/dictionary/v2"));
			n_env.PutAttrib(SXml::nst("xmlns", "ikar"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar"));
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst("ws", "getRussianEnterpriseListRequest"));
					PutListOptionsParam(srb, offs, count);
				}
			}
		}
		xmlTextWriterFlush(srb);
		reply_buf.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
		THROW(SendSOAP(InetUrl::MkHttps("api.vetrf.ru", "platform/cerberus/services/EnterpriseService"), "GetRussianEnterpriseList", reply_buf, temp_buf));
		THROW(ParseReply(temp_buf, rReply));
	}
    CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetBusinessEntityList(uint offs, uint count, VetisApplicationBlock & rReply)
{
	int    ok = -1;
	SString temp_buf;
	SString reply_buf;
	rReply.Clear();
	THROW(State & stInited);
	{
		VetisSubmitRequestBlock srb;
		{
			SXml::WNode n_env(srb, SXml::nst("soapenv", "Envelope"));
			n_env.PutAttrib(SXml::nst("xmlns", "soapenv"), InetUrl::MkHttp("schemas.xmlsoap.org", "soap/envelope/"));
			n_env.PutAttrib(SXml::nst("xmlns", "ws"), "http://api.vetrf.ru/schema/cdm/cerberus/business-entity/ws-definitions");
			n_env.PutAttrib(SXml::nst("xmlns", "base"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/base"));
			n_env.PutAttrib(SXml::nst("xmlns", "ent"),  InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/cerberus/enterprise"));
			n_env.PutAttrib(SXml::nst("xmlns", "ikar"), InetUrl::MkHttp("api.vetrf.ru", "schema/cdm/ikar"));
			{
				SXml::WNode n_hdr(srb, SXml::nst("soapenv", "Header"));
			}
			{
				SXml::WNode n_bdy(srb, SXml::nst("soapenv", "Body"));
				{
					SXml::WNode n_f(srb, SXml::nst("ws", "getBusinessEntityListRequest"));
					PutListOptionsParam(srb, offs, count);
				}
			}
		}
		xmlTextWriterFlush(srb);
		reply_buf.CopyFromN((char *)((xmlBuffer *)srb)->content, ((xmlBuffer *)srb)->use);
		THROW(SendSOAP(InetUrl::MkHttps("api.vetrf.ru", "platform/cerberus/services/EnterpriseService"), "GetBusinessEntityList", reply_buf, temp_buf));
		THROW(ParseReply(temp_buf, rReply));
	}
    CATCHZOK
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
		THROW(ReceiveResult(submit_result.ApplicationId, receive_result));
	}
	*/
	ok = -1;
	//CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::PrepareApplicationBlockForReq(VetisApplicationBlock & rBlk)
{
	int   ok = 1;
	SString temp_buf;
	P.GetExtStrData(extssQInitiator, temp_buf);
	rBlk.User = temp_buf;
	rBlk.ServiceId = "mercury-g2b.service"; // "mercury-g2b.service:2.0";
	if(rBlk.VetisSvcVer == 2)
		rBlk.ServiceId.Cat(":2.0");
	rBlk.IssuerId = P.IssuerUUID;
	rBlk.EnterpriseId = P.EntUUID;
	rBlk.IssueDate = getcurdatetime_();
	rBlk.ApplicationId.Generate();
	rBlk.LocalTransactionId = ++LastLocalTransactionId;
	return ok;
}

int SLAPI PPVetisInterface::GetVetDocumentByUuid(const S_GUID & rUuid, VetisApplicationBlock & rReply)
{
	int    ok = -1;
	VetisGetVetDocumentByUuidRequest app_data;
	VetisApplicationBlock submit_result;
	VetisApplicationBlock blk(1, &app_data);
	rReply.Clear();
	THROW(PrepareApplicationBlockForReq(blk));
	app_data.Uuid = rUuid;
	THROW(SubmitRequest(blk, submit_result));
	if(submit_result.ApplicationStatus == VetisApplicationBlock::appstAccepted) {
		THROW(ReceiveResult(submit_result.ApplicationId, rReply));
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetVetDocumentChangesList(const STimeChunk & rPeriod, uint startOffset, uint count, VetisApplicationBlock & rReply)
{
	int    ok = -1;
	VetisGetVetDocumentChangesListRequest app_data;
	VetisApplicationBlock submit_result;
	VetisApplicationBlock blk(1, &app_data);
	rReply.Clear();
	THROW(PrepareApplicationBlockForReq(blk));
	app_data.ListOptions.Set(startOffset, count);
	app_data.Period = rPeriod;
	THROW(SubmitRequest(blk, submit_result));
	if(submit_result.ApplicationStatus == VetisApplicationBlock::appstAccepted) {
		THROW(ReceiveResult(submit_result.ApplicationId, rReply));
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetVetDocumentList(uint startOffset, uint count, VetisApplicationBlock & rReply)
{
	int    ok = -1;
	VetisGetVetDocumentListRequest app_data;
	VetisApplicationBlock submit_result;
	VetisApplicationBlock blk(1, &app_data);
	rReply.Clear();
	THROW(PrepareApplicationBlockForReq(blk));
	app_data.ListOptions.Set(startOffset, count);
	THROW(SubmitRequest(blk, submit_result));
	if(submit_result.ApplicationStatus == VetisApplicationBlock::appstAccepted) {
		THROW(ReceiveResult(submit_result.ApplicationId, rReply));
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::ProcessIncomingConsignment(const S_GUID & rDocUuid, VetisApplicationBlock & rReply)
{	
	int    ok = -1;
	VetisApplicationBlock doc_reply;
	if(GetVetDocumentByUuid(rDocUuid, doc_reply) > 0 && doc_reply.VetDocList.getCount() == 1) {
		VetisProcessIncomingConsignmentRequest app_data;
		VetisApplicationBlock submit_result;
		VetisApplicationBlock blk(1, &app_data);
		rReply.Clear();
		THROW(PrepareApplicationBlockForReq(blk));
		app_data.Doc = *doc_reply.VetDocList.at(0);
		THROW(SubmitRequest(blk, submit_result));
		if(submit_result.ApplicationStatus == VetisApplicationBlock::appstAccepted) {
			THROW(ReceiveResult(submit_result.ApplicationId, rReply));
			ok = 1;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI PPVetisInterface::GetStockEntryList(uint startOffset, uint count)
{
	int    ok = 1;
	SString temp_buf;
	VetisApplicationData app_data(VetisApplicationData::signGetStockEntryList);
	VetisApplicationBlock submit_result;
	VetisApplicationBlock receive_result;
	VetisApplicationBlock blk(1, &app_data);
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
		THROW(ReceiveResult(submit_result.ApplicationId, receive_result));
	}
	CATCHZOK
	return ok;
}

static void Debug_OutputProductItem(const VetisProductItem & rItem, SString & rBuf)
{
	rBuf.Z();
	rBuf.Cat(rItem.Uuid).Tab().Cat(rItem.Guid);
	rBuf.Tab().Cat(rItem.GlobalID);
	rBuf.Tab().Cat(rItem.Code);
	rBuf.Tab().Cat(rItem.Name);
	rBuf.Tab().Cat(rItem.ProductType);
	rBuf.Tab().CatHex(rItem.Flags);
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

static void Debug_OutputBEntItem(const VetisBusinessEntity & rItem, SString & rBuf)
{
	SString temp_buf;
	rBuf.Z();
	rBuf.Cat(rItem.Uuid).Tab().Cat(rItem.Guid);
	rBuf.Tab().Cat(rItem.Name).Tab().Cat(rItem.Fio).Tab().Cat(rItem.Inn).Tab().Cat(rItem.Kpp);
	rBuf.Tab().Cat(rItem.JuridicalAddress.AddressView);
}

int SLAPI PPVetisInterface::ProcessUnresolvedEntityList(TSVector <VetisEntityCore::UnresolvedEntity> & rList)
{
	int    ok = 1;
	SString temp_buf;
	for(uint i = 0; i < rList.getCount(); i++) {
		VetisEntityCore::UnresolvedEntity & r_entry = rList.at(i);
		VetisApplicationBlock reply(1, 0);
		S_GUID guid;
		int    rr = 1;
		PPID   entity_id = 0;
		if(r_entry.Kind == VetisEntityCore::kProductItem) {
			if(r_entry.GuidRef && PeC.UrT.Search(r_entry.GuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtProductItemByGuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			else if(r_entry.UuidRef && PeC.UrT.Search(r_entry.UuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtProductItemByUuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			THROW(rr);
			if(rr > 0) {
				for(uint j = 0; j < reply.ProductItemList.getCount(); j++) {
					THROW(PeC.Put(&entity_id, r_entry.Kind, *reply.ProductItemList.at(j), 0, 1));
				}
			}
		}
		else if(r_entry.Kind == VetisEntityCore::kProduct) {
			if(r_entry.GuidRef && PeC.UrT.Search(r_entry.GuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtProductByGuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			else if(r_entry.UuidRef && PeC.UrT.Search(r_entry.UuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtProductByUuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			THROW(rr);
		}
		else if(r_entry.Kind == VetisEntityCore::kSubProduct) {
			if(r_entry.GuidRef && PeC.UrT.Search(r_entry.GuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtSubProductByGuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			else if(r_entry.UuidRef && PeC.UrT.Search(r_entry.UuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtSubProductByUuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			THROW(rr);
		}
		else if(r_entry.Kind == VetisEntityCore::kEnterprise) {
			if(r_entry.GuidRef && PeC.UrT.Search(r_entry.GuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtEnterpriseByGuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			else if(r_entry.UuidRef && PeC.UrT.Search(r_entry.UuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtEnterpriseByUuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			THROW(rr);
			if(rr > 0) {
				for(uint j = 0; j < reply.EntItemList.getCount(); j++) {
					THROW(PeC.Put(&entity_id, *reply.EntItemList.at(j), 0, 1));
				}
			}
		}
		else if(r_entry.Kind == VetisEntityCore::kBusinessEntity) {
			if(r_entry.GuidRef && PeC.UrT.Search(r_entry.GuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtBusinessEntityByGuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			else if(r_entry.UuidRef && PeC.UrT.Search(r_entry.UuidRef, guid) > 0 && !guid.IsZero())
				rr = GetEntityQuery(qtBusinessEntityByUuid, guid.ToStr(S_GUID::fmtIDL|S_GUID::fmtLower, temp_buf), reply);
			THROW(rr);
			if(rr > 0) {
				for(uint j = 0; j < reply.BEntList.getCount(); j++) {
					THROW(PeC.Put(&entity_id, *reply.BEntList.at(j), 0, 1));
				}
			}
		}
		PPWaitPercent(i+1, rList.getCount(), 0);
	}
	CATCHZOK
	return ok;
}

int SLAPI TestVetis()
{
	int    ok = 1;
	SString temp_buf;
	PPLogger logger;
	PPObjGoods goods_obj;
	PPVetisInterface::Param param(0, 0);
	TSVector <VetisEntityCore::UnresolvedEntity> ure_list;
	THROW(PPVetisInterface::SetupParam(param));
	PPWait(1);
	{
		PPVetisInterface ifc(&logger);
		THROW(ifc.Init(param));
		/*{
			THROW(ifc.PeC.CollectUnresolvedEntityList(ure_list));
			THROW(ifc.ProcessUnresolvedEntityList(ure_list));
			ure_list.clear();
		}*/
		//THROW(ifc.GetStockEntryList());
		//THROW(ifc.GetAppliedUserAuthorityList());
		{
			//ifc.GetAppliedUserAuthorityList();
		}
#if 0 // {
		{
			VetisApplicationBlock reply(1, 0);
			uint req_count = 1000;
			for(uint req_offs = 0; ifc.GetVetDocumentList(req_offs, req_count, reply);) {
				PPTransaction tra(1);
				THROW(tra);
				for(uint i = 0; i < reply.VetDocList.getCount(); i++) {
					const VetisVetDocument * p_item = reply.VetDocList.at(i);
					if(p_item) {
						PPID   pi_id = 0;
						THROW(ifc.PeC.Put(&pi_id, *p_item, &ure_list, 0));
					}
				}
				THROW(tra.Commit());
				PPWaitPercent(reply.ListResult.Count+reply.ListResult.Offset, reply.ListResult.Total);
				if(reply.VetDocList.getCount() < req_count)
					break;
				else
					req_offs += reply.VetDocList.getCount();
			}
			THROW(ifc.ProcessUnresolvedEntityList(ure_list));
		}
		{
			VetisApplicationBlock reply;
			temp_buf = "fcbef59e-218a-11e2-a69b-b499babae7ea"; // ЗАО "Митлэнд inn 7816358675
			ifc.GetEntityQuery(ifc.qtBusinessEntityByGuid, temp_buf, reply);
		}
		{
			VetisApplicationBlock reply;
			temp_buf = "04ac940e-053d-11e1-99b4-d8d385fbc9e8"; // ЗАО "Митлэнд inn 7816358675
			ifc.GetEntityQuery(ifc.qtBusinessEntityByUuid, temp_buf, reply);
		}
		{
			VetisApplicationBlock reply;
			temp_buf = "1c44b8ce-4e22-f1ee-0446-1844351a5838"; // ООО "Тельпас"
			ifc.GetEntityQuery(ifc.qtEnterpriseByGuid, temp_buf, reply);
		}
		{
			VetisApplicationBlock reply;
			temp_buf = "075985c2-053d-11e1-99b4-d8d385fbc9e8"; // ООО "Тельпас"
			ifc.GetEntityQuery(ifc.qtEnterpriseByUuid, temp_buf, reply);
		}
		{
			VetisApplicationBlock reply;
			temp_buf = "69bf7cfb-fd32-4d2b-99d6-7629dc4ad040"; // Колбаса "Любительская" вареная
			ifc.GetEntityQuery(ifc.qtProductItemByGuid, temp_buf, reply);
		}
		{
			VetisApplicationBlock reply;
			temp_buf = "aceefd38-40c4-4276-96ad-039ed113a156"; // Колбаса "Любительская" вареная
			ifc.GetEntityQuery(ifc.qtProductItemByUuid, temp_buf, reply);
		}
#endif // } 0
		{
			const uint max_zeroresult_tries = 3;
			uint  zeroresult_try = 0;
			PPGetFilePath(PPPATH_LOG, "vetis_bent_list.log", temp_buf);
			SFile f_out(temp_buf, SFile::mWrite);
			uint req_count = 1000;
			VetisApplicationBlock reply;
			for(uint req_offs = 0; ifc.GetBusinessEntityList(req_offs, req_count, reply);) {
				if(reply.BEntList.getCount() == 0) {
					if(zeroresult_try < max_zeroresult_tries)
						zeroresult_try++;
					else
						break;
				}
				else {
					PPTransaction tra(1);
					THROW(tra);
					for(uint i = 0; i < reply.BEntList.getCount(); i++) {
						const VetisBusinessEntity * p_item = reply.BEntList.at(i);
						if(p_item) {
							Debug_OutputBEntItem(*p_item, temp_buf);
							f_out.WriteLine(temp_buf.CR());
							PPID   pi_id = 0;
							THROW(ifc.PeC.Put(&pi_id, *p_item, &ure_list, 0));
						}
					}
					THROW(tra.Commit());
					PPWaitPercent(reply.ListResult.Count+reply.ListResult.Offset, reply.ListResult.Total);
					zeroresult_try = 0;
					if(reply.BEntList.getCount() < req_count)
						break;
					else
						req_offs += reply.BEntList.getCount();
				}
			}
		}
#if 0 // {
		{
			VetisApplicationBlock reply;
			PPGetFilePath(PPPATH_LOG, "vetis_product_list.log", temp_buf);
			SFile f_out(temp_buf, SFile::mWrite);
			uint req_count = 1000;
			for(uint req_offs = 0; ifc.GetProductItemList(req_offs, req_count, reply);) {
				PPTransaction tra(1);
				THROW(tra);
				for(uint i = 0; i < reply.ProductItemList.getCount(); i++) {
					VetisProductItem * p_item = reply.ProductItemList.at(i);
					if(p_item) {
						if(p_item->GlobalID.NotEmpty()) {
							BarcodeTbl::Rec bc_rec;
							if(goods_obj.SearchByBarcode(p_item->GlobalID, &bc_rec, 0, 1) > 0)
								p_item->NativeGoodsID = bc_rec.GoodsID;
						}
						Debug_OutputProductItem(*p_item, temp_buf);
						f_out.WriteLine(temp_buf.CR());
						PPID   pi_id = 0;
						THROW(ifc.PeC.Put(&pi_id, ifc.PeC.kProductItem, *p_item, &ure_list, 0));
					}
				}
				THROW(tra.Commit());
				PPWaitPercent(reply.ListResult.Count+reply.ListResult.Offset, reply.ListResult.Total);
				if(reply.ProductItemList.getCount() < req_count)
					break;
				else
					req_offs += reply.ProductItemList.getCount();
			}
		}
		{
			PPGetFilePath(PPPATH_LOG, "vetis_ent_list.log", temp_buf);
			SFile f_out(temp_buf, SFile::mWrite);
			uint req_count = 1000;
			VetisApplicationBlock reply;
			for(uint req_offs = 0; ifc.GetRussianEnterpriseList(req_offs, req_count, reply);) {
				PPTransaction tra(1);
				THROW(tra);
				for(uint i = 0; i < reply.EntItemList.getCount(); i++) {
					const VetisEnterprise * p_item = reply.EntItemList.at(i);
					if(p_item) {
						Debug_OutputEntItem(*p_item, temp_buf);
						f_out.WriteLine(temp_buf.CR());
						PPID   pi_id = 0;
						THROW(ifc.PeC.Put(&pi_id, *p_item, &ure_list, 0));
					}
				}
				THROW(tra.Commit());
				PPWaitPercent(reply.ListResult.Count+reply.ListResult.Offset, reply.ListResult.Total);
				if(reply.EntItemList.getCount() < req_count)
					break;
				else
					req_offs += reply.EntItemList.getCount();
			}
		}
#endif // } 0
	}
	CATCHZOKPPERR
	PPWait(0);
	return ok;
}

int EditVetisVetDocument(VetisVetDocument & rData)
{
	int    ok = -1;
	TDialog * dlg = new TDialog(DLG_VETVDOC);
	if(CheckDialogPtrErr(&dlg)) {
		VetisCertifiedConsignment & r_crtc = rData.CertifiedConsignment;
		SString temp_buf;
		SString text_buf;
		//dlg->setCtrlLong(rData.)
		dlg->setCtrlString(CTL_VETVDOC_UUID, temp_buf.Z().Cat(rData.Uuid, S_GUID::fmtIDL|S_GUID::fmtLower));
		dlg->setCtrlDate(CTL_VETVDOC_DT, rData.IssueDate);
		dlg->setCtrlString(CTL_VETVDOC_NUM, rData.IssueNumber);
		dlg->setCtrlDate(CTL_VETVDOC_WBDT, rData.WayBillDate);
		dlg->setCtrlString(CTL_VETVDOC_WBNUM, rData.WayBillNumber);
		SIntToSymbTab_GetSymb(VetisVetDocStatus_SymbTab, SIZEOFARRAY(VetisVetDocStatus_SymbTab), rData.VetDStatus, temp_buf);
		dlg->setCtrlString(CTL_VETVDOC_VDSTATUS, temp_buf);
		SIntToSymbTab_GetSymb(VetisVetDocFormat_SymbTab, SIZEOFARRAY(VetisVetDocFormat_SymbTab), rData.VetDForm, temp_buf);
		dlg->setCtrlString(CTL_VETVDOC_VDFORM, temp_buf);
		SIntToSymbTab_GetSymb(VetisVetDocType_SymbTab, SIZEOFARRAY(VetisVetDocType_SymbTab), rData.VetDType, temp_buf);
		dlg->setCtrlString(CTL_VETVDOC_VDTYPE, temp_buf);
		dlg->setCtrlReal(CTL_VETVDOC_QTTY, r_crtc.Batch.Volume);

		text_buf.Z();
		if(!!r_crtc.Consignor.Enterprise.Guid) {
			temp_buf.Z().Cat(r_crtc.Consignor.Enterprise.Guid, S_GUID::fmtIDL|S_GUID::fmtLower);
			text_buf.CatDivIfNotEmpty(';', 2).Cat(temp_buf);
		}
		if(!!r_crtc.Consignor.BusinessEntity.Guid) {
			temp_buf.Z().Cat(r_crtc.Consignor.BusinessEntity.Guid, S_GUID::fmtIDL|S_GUID::fmtLower);
			text_buf.CatDivIfNotEmpty(';', 2).Cat(temp_buf);
		}
		dlg->setCtrlString(CTL_VETVDOC_FROMSI, text_buf);
		if(r_crtc.Consignor.Enterprise.Name.NotEmpty())
			dlg->setCtrlString(CTL_VETVDOC_FROMNAM, r_crtc.Consignor.Enterprise.Name);
		else
			dlg->setCtrlString(CTL_VETVDOC_FROMNAM, r_crtc.Consignor.BusinessEntity.Name);

		text_buf.Z();
		if(!!r_crtc.Consignee.Enterprise.Guid) {
			temp_buf.Z().Cat(r_crtc.Consignee.Enterprise.Guid, S_GUID::fmtIDL|S_GUID::fmtLower);
			text_buf.CatDivIfNotEmpty(';', 2).Cat(temp_buf);
		}
		if(!!r_crtc.Consignee.BusinessEntity.Guid) {
			temp_buf.Z().Cat(r_crtc.Consignee.BusinessEntity.Guid, S_GUID::fmtIDL|S_GUID::fmtLower);
			text_buf.CatDivIfNotEmpty(';', 2).Cat(temp_buf);
		}
		dlg->setCtrlString(CTL_VETVDOC_TOSI, text_buf);
		if(r_crtc.Consignee.Enterprise.Name.NotEmpty())
			dlg->setCtrlString(CTL_VETVDOC_TONAM, r_crtc.Consignee.Enterprise.Name);
		else
			dlg->setCtrlString(CTL_VETVDOC_TONAM, r_crtc.Consignee.BusinessEntity.Name);

		text_buf.Z();
		if(!!r_crtc.Batch.Product.Guid) {
			temp_buf.Z().Cat(r_crtc.Batch.Product.Guid, S_GUID::fmtIDL|S_GUID::fmtLower);
			text_buf.CatDivIfNotEmpty(';', 2).Cat(temp_buf);
		}
		dlg->setCtrlString(CTL_VETVDOC_PRODSI, text_buf);
		dlg->setCtrlString(CTL_VETVDOC_PRODNAM, r_crtc.Batch.Product.Name);

		text_buf.Z();
		if(!!r_crtc.Batch.SubProduct.Guid) {
			temp_buf.Z().Cat(r_crtc.Batch.SubProduct.Guid, S_GUID::fmtIDL|S_GUID::fmtLower);
			text_buf.CatDivIfNotEmpty(';', 2).Cat(temp_buf);
		}
		dlg->setCtrlString(CTL_VETVDOC_SPRODSI, text_buf);
		dlg->setCtrlString(CTL_VETVDOC_SPRODNAM, r_crtc.Batch.SubProduct.Name);

		text_buf.Z();
		if(!!r_crtc.Batch.ProductItem.Guid) {
			temp_buf.Z().Cat(r_crtc.Batch.ProductItem.Guid, S_GUID::fmtIDL|S_GUID::fmtLower);
			text_buf.CatDivIfNotEmpty(';', 2).Cat(temp_buf);
		}
		dlg->setCtrlString(CTL_VETVDOC_ITEMSI, text_buf);
		dlg->setCtrlString(CTL_VETVDOC_ITEMNAM, r_crtc.Batch.ProductItem.Name);
		if(r_crtc.Batch.NativeGoodsID) {
			GetGoodsName(r_crtc.Batch.NativeGoodsID, temp_buf);
			dlg->setCtrlString(CTL_VETVDOC_ITEMLNK, temp_buf);
		}
		ExecView(dlg);
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}

IMPLEMENT_PPFILT_FACTORY(VetisDocument); SLAPI VetisDocumentFilt::VetisDocumentFilt() : PPBaseFilt(PPFILT_VETISDOCUMENT, 0, 1)
{
	SetFlatChunk(offsetof(VetisDocumentFilt, ReserveStart),
		offsetof(VetisDocumentFilt, ReserveEnd) - offsetof(VetisDocumentFilt, ReserveStart));
	Init(1, 0);
}

int FASTCALL VetisDocumentFilt::GetStatusList(PPIDArray & rList) const
{
	rList.clear();
	if(VDStatusFlags) {
		if(VDStatusFlags & (1<<vetisdocstCREATED))
			rList.add(vetisdocstCREATED);
		if(VDStatusFlags & (1<<vetisdocstCONFIRMED))
			rList.add(vetisdocstCONFIRMED);
		if(VDStatusFlags & (1<<vetisdocstWITHDRAWN))
			rList.add(vetisdocstWITHDRAWN);
		if(VDStatusFlags & (1<<vetisdocstUTILIZED))
			rList.add(vetisdocstUTILIZED);
		if(VDStatusFlags & (1<<vetisdocstFINALIZED))
			rList.add(vetisdocstFINALIZED);
		return 1;
	}
	else
		return -1;
}

SLAPI PPViewVetisDocument::PPViewVetisDocument() : PPView(0, &Filt, PPVIEW_VETISDOCUMENT)
{
}

SLAPI PPViewVetisDocument::~PPViewVetisDocument()
{
}

class VetisDocumentFiltDialog : public TDialog {
public:
	VetisDocumentFiltDialog() : TDialog(DLG_VETDOCFLT)
	{
		SetupCalPeriod(CTLCAL_VETDOCFLT_PRD, CTL_VETDOCFLT_PRD);
		SetupCalPeriod(CTLCAL_VETDOCFLT_WBPRD, CTL_VETDOCFLT_WBPRD);
	}
	int    setDTS(const VetisDocumentFilt * pData)
	{
		int    ok = 1;
		RVALUEPTR(Data, pData);
		SetPeriodInput(this, CTL_VETDOCFLT_PRD,   &Data.Period);
		SetPeriodInput(this, CTL_VETDOCFLT_WBPRD, &Data.WayBillPeriod);
		AddClusterAssoc(CTL_VETDOCFLT_STATUS, 0, (1<<vetisdocstCREATED));
		AddClusterAssoc(CTL_VETDOCFLT_STATUS, 1, (1<<vetisdocstCONFIRMED));
		AddClusterAssoc(CTL_VETDOCFLT_STATUS, 2, (1<<vetisdocstWITHDRAWN));
		AddClusterAssoc(CTL_VETDOCFLT_STATUS, 3, (1<<vetisdocstUTILIZED));
		AddClusterAssoc(CTL_VETDOCFLT_STATUS, 4, (1<<vetisdocstFINALIZED));
		SetClusterData(CTL_VETDOCFLT_STATUS, Data.VDStatusFlags);
		SetupLocationCombo(this, CTLSEL_VETDOCFLT_WH, Data.LocID, 0, LOCTYP_WAREHOUSE, 0);
		return ok;
	}
	int    getDTS(VetisDocumentFilt * pData)
	{
		int    ok = 1;
		GetPeriodInput(this, CTL_VETDOCFLT_PRD,   &Data.Period);
		GetPeriodInput(this, CTL_VETDOCFLT_WBPRD, &Data.WayBillPeriod);
		GetClusterData(CTL_VETDOCFLT_STATUS, &Data.VDStatusFlags);
		getCtrlData(CTLSEL_VETDOCFLT_WH, &Data.LocID);
		ASSIGN_PTR(pData, Data);
		return ok;
	}
private:
	VetisDocumentFilt Data;
};

int SLAPI PPViewVetisDocument::EditBaseFilt(PPBaseFilt * pBaseFilt)
{
	if(!Filt.IsA(pBaseFilt))
		return PPErrorZ();
	VetisDocumentFilt * p_filt = (VetisDocumentFilt *)pBaseFilt;
	DIALOG_PROC_BODY(VetisDocumentFiltDialog, p_filt);
}

int SLAPI PPViewVetisDocument::Init_(const PPBaseFilt * pBaseFilt)
{
	int    ok = 1;
	BExtQuery::ZDelete(&P_IterQuery);
	THROW(Helper_InitBaseFilt(pBaseFilt));
	CATCHZOK
	return ok;
}

static IMPL_DBE_PROC(dbqf_vetis_clsftextfld_iisi)
{
	char   result_buf[32];
	if(option == CALC_SIZE) {
		result->init((long)sizeof(result_buf));
	}
	else {
		PPID   _id = params[0].lval;
		//PPID    = params[1].lval;
		SString temp_buf;
		STRNSCPY(result_buf, temp_buf);
		result->init(result_buf);
	}
}

static IMPL_DBE_PROC(dbqf_vetis_entitytextfld_ip)
{
	char   result_buf[128];
	if(option == CALC_SIZE) {
		result->init((long)sizeof(result_buf));
	}
	else {
		Reference * p_ref = PPRef;
		PPID   entity_id = params[0].lval;
		VetisEntityCore * p_ec = (VetisEntityCore *)params[1].ptrv;
		SString temp_buf;
		SStringU temp_buf_u;
		if(entity_id) {
			TextRefIdent tri(PPOBJ_VETISENTITY, entity_id, PPTRPROP_NAME);
			p_ref->TrT.Search(tri, temp_buf_u);
		}
		if(temp_buf_u.Len()) {
			temp_buf_u.CopyToUtf8(temp_buf, 0);
			temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		STRNSCPY(result_buf, temp_buf);
		result->init(result_buf);
	}
}

static IMPL_DBE_PROC(dbqf_vetis_businessmembtextfld_iip)
{
	char   result_buf[128];
	if(option == CALC_SIZE) {
		result->init((long)sizeof(result_buf));
	}
	else {
		Reference * p_ref = PPRef;
		PPID   enterprise_id = params[0].lval;
		PPID   bent_id = params[1].lval;
		VetisEntityCore * p_ec = (VetisEntityCore *)params[2].ptrv;
		SString temp_buf;
		SStringU temp_buf_u;
		if(bent_id) {
			TextRefIdent tri(PPOBJ_VETISENTITY, bent_id, PPTRPROP_NAME);
			p_ref->TrT.Search(tri, temp_buf_u);
		}
		if(!temp_buf_u.Len() && enterprise_id) {
			TextRefIdent tri(PPOBJ_VETISENTITY, enterprise_id, PPTRPROP_NAME);
			p_ref->TrT.Search(tri, temp_buf_u);
		}
		if(temp_buf_u.Len()) {
			temp_buf_u.CopyToUtf8(temp_buf, 0);
			temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		STRNSCPY(result_buf, temp_buf);
		result->init(result_buf);
	}
}

static IMPL_DBE_PROC(dbqf_vetis_productitemtextfld_iiiip)
{
	char   result_buf[128];
	if(option == CALC_SIZE) {
		result->init((long)sizeof(result_buf));
	}
	else {
		Reference * p_ref = PPRef;
		PPID   product_item_id = params[0].lval;
		PPID   subproduct_id = params[1].lval;
		PPID   product_id = params[2].lval;
		PPID   entity_id = params[3].lval;
		VetisEntityCore * p_ec = (VetisEntityCore *)params[2].ptrv;
		SString temp_buf;
		SStringU temp_buf_u;
		if(product_item_id) {
			TextRefIdent tri(PPOBJ_VETISENTITY, product_item_id, PPTRPROP_NAME);
			p_ref->TrT.Search(tri, temp_buf_u);
		}
		if(temp_buf_u.Len() == 0 && entity_id) {
			TextRefIdent tri(PPOBJ_VETISENTITY, entity_id, VetisEntityCore::txtprpProductItemName);
			p_ref->UtrC.Search(tri, temp_buf_u);
		}
		if(temp_buf_u.Len() == 0 && subproduct_id) {
			TextRefIdent tri(PPOBJ_VETISENTITY, subproduct_id, PPTRPROP_NAME);
			p_ref->TrT.Search(tri, temp_buf_u);
		}
		if(temp_buf_u.Len() == 0 && product_id) {
			TextRefIdent tri(PPOBJ_VETISENTITY, product_id, PPTRPROP_NAME);
			p_ref->TrT.Search(tri, temp_buf_u);
		}
		if(temp_buf_u.Len()) {
			temp_buf_u.CopyToUtf8(temp_buf, 0);
			temp_buf.Transf(CTRANSF_UTF8_TO_INNER);
		}
		STRNSCPY(result_buf, temp_buf);
		result->init(result_buf);
	}
}

static IMPL_DBE_PROC(dbqf_vetis_vetdstatus_i)
{
	char   buf[32];
	if(option == CALC_SIZE) {
		result->init((long)sizeof(buf));
	}
	else {
		SString temp_buf;
		SIntToSymbTab_GetSymb(VetisVetDocStatus_SymbTab, SIZEOFARRAY(VetisVetDocStatus_SymbTab), params[0].lval, temp_buf);
		temp_buf.CopyTo(buf, sizeof(buf));
		result->init(buf);
	}
}

static IMPL_DBE_PROC(dbqf_vetis_vetdform_i)
{
	char   buf[32];
	if(option == CALC_SIZE) {
		result->init((long)sizeof(buf));
	}
	else {
		SString temp_buf;
		SIntToSymbTab_GetSymb(VetisVetDocFormat_SymbTab, SIZEOFARRAY(VetisVetDocFormat_SymbTab), params[0].lval, temp_buf);
		temp_buf.CopyTo(buf, sizeof(buf));
		result->init(buf);
	}
}

static IMPL_DBE_PROC(dbqf_vetis_vetdtype_i)
{
	char   buf[32];
	if(option == CALC_SIZE) {
		result->init((long)sizeof(buf));
	}
	else {
		SString temp_buf;
		SIntToSymbTab_GetSymb(VetisVetDocType_SymbTab, SIZEOFARRAY(VetisVetDocType_SymbTab), params[0].lval, temp_buf);
		temp_buf.CopyTo(buf, sizeof(buf));
		result->init(buf);
	}
}

int PPViewVetisDocument::DynFuncBMembTextFld = 0;
int PPViewVetisDocument::DynFuncEntityTextFld = 0;
int PPViewVetisDocument::DynFuncProductItemTextFld = 0;
int PPViewVetisDocument::DynFuncVetDStatus = 0;
int PPViewVetisDocument::DynFuncVetDForm = 0;
int PPViewVetisDocument::DynFuncVetDType = 0;

DBQuery * SLAPI PPViewVetisDocument::CreateBrowserQuery(uint * pBrwId, SString * pSubTitle)
{
	DbqFuncTab::RegisterDyn(&DynFuncEntityTextFld, BTS_STRING, dbqf_vetis_entitytextfld_ip, 2, BTS_INT, BTS_PTR);
	DbqFuncTab::RegisterDyn(&DynFuncBMembTextFld, BTS_STRING, dbqf_vetis_businessmembtextfld_iip, 3, BTS_INT, BTS_INT, BTS_PTR);
	DbqFuncTab::RegisterDyn(&DynFuncProductItemTextFld, BTS_STRING, dbqf_vetis_productitemtextfld_iiiip, 5, BTS_INT, BTS_INT, BTS_INT, BTS_INT, BTS_PTR);
	DbqFuncTab::RegisterDyn(&DynFuncVetDStatus, BTS_STRING, dbqf_vetis_vetdstatus_i, 1, BTS_INT);
	DbqFuncTab::RegisterDyn(&DynFuncVetDForm, BTS_STRING, dbqf_vetis_vetdform_i, 1, BTS_INT);
	DbqFuncTab::RegisterDyn(&DynFuncVetDType, BTS_STRING, dbqf_vetis_vetdtype_i, 1, BTS_INT);

	uint   brw_id = BROWSER_VETISDOCUMENT;
	DBQ     * dbq = 0;
	DBQuery * q = 0;
	VetisDocumentTbl * t = 0;
	PPIDArray status_list;
	DBE    dbe_product_name;
	DBE    dbe_vetdform;
	DBE    dbe_vetdtype;
	DBE    dbe_vetdstatus;
	DBE    dbe_from;
	DBE    dbe_to;
	THROW(CheckTblPtr(t = new VetisDocumentTbl(EC.DT.GetName())));
	{
		dbe_product_name.init();
		dbe_product_name.push(t->ProductItemID);
		dbe_product_name.push(t->SubProductID);
		dbe_product_name.push(t->ProductID);
		dbe_product_name.push(t->EntityID);
		{
			DBConst cp;
			cp.init(&EC.DT);
			dbe_product_name.push(cp);
		}
		dbe_product_name.push((DBFunc)DynFuncProductItemTextFld);
	}
	{
		dbe_vetdform.init();
		dbe_vetdform.push(t->VetisDocForm);
		dbe_vetdform.push((DBFunc)DynFuncVetDForm);
	}
	{
		dbe_vetdtype.init();
		dbe_vetdtype.push(t->VetisDocType);
		dbe_vetdtype.push((DBFunc)DynFuncVetDType);
	}
	{
		dbe_vetdstatus.init();
		dbe_vetdstatus.push(t->VetisDocStatus);
		dbe_vetdstatus.push((DBFunc)DynFuncVetDStatus);
	}
	/*{
		dbe_from.init();
		dbe_from.push(t->FromEnterpriseID);
		dbe_from.push((DBFunc)DynFuncEntityTextFld);
	}
	{
		dbe_to.init();
		dbe_to.push(t->ToEnterpriseID);
		dbe_to.push((DBFunc)DynFuncEntityTextFld);
	}*/
	//
	{
		dbe_from.init();
		dbe_from.push(t->FromEnterpriseID);
		dbe_from.push(t->FromEntityID);
		dbe_from.push((DBFunc)DynFuncBMembTextFld);
	}
	{
		dbe_to.init();
		dbe_to.push(t->ToEnterpriseID);
		dbe_to.push(t->ToEntityID);
		dbe_to.push((DBFunc)DynFuncBMembTextFld);
	}
	dbq = & (*dbq && daterange(t->IssueDate, &Filt.Period));
	dbq = & (*dbq && daterange(t->WayBillDate, &Filt.WayBillPeriod));
	if(Filt.GetStatusList(status_list) > 0)
		dbq = & (*dbq && ppidlist(t->VetisDocStatus, &status_list));
	q = &select(
		t->EntityID,          //  #0
		t->Flags,             //  #1
		t->LinkBillID,        //  #2
		t->LinkBillRow,       //  #3 
		t->LinkGoodsID,       //  #4 
		t->LinkFromPsnID,     //  #5 
		t->LinkFromDlvrLocID, //  #6 
		t->LinkToPsnID,       //  #7
		t->LinkToDlvrLocID,   //  #8 
		t->IssueDate,         //  #9
		dbe_vetdform,         //  #10
		dbe_vetdtype,         //  #11
		dbe_vetdstatus,       //  #12
		t->WayBillDate,       //  #13
		t->WayBillNumber,     //  #14
		dbe_product_name,     //  #15
		t->Volume,            //  #16
		dbe_from,             //  #17
		dbe_to,               //  #18
		0L).from(t, 0).where(*dbq);
	CATCH
		if(q)
			ZDELETE(q);
		else
			ZDELETE(t);
	ENDCATCH
	ASSIGN_PTR(pBrwId, brw_id);
	return q;
}

static int CellStyleFunc(const void * pData, long col, int paintAction, BrowserWindow::CellStyle * pStyle, void * extraPtr)
{
	int    ok = -1;
	PPViewBrowser * p_brw = (PPViewBrowser *)extraPtr;
	if(p_brw) {
		PPViewVetisDocument * p_view = (PPViewVetisDocument *)p_brw->P_View;
		ok = p_view ? p_view->CellStyleFunc_(pData, col, paintAction, pStyle, p_brw) : -1;
	}
	return ok;
}

int SLAPI PPViewVetisDocument::CellStyleFunc_(const void * pData, long col, int paintAction, BrowserWindow::CellStyle * pStyle, PPViewBrowser * pBrw)
{
	int    ok = -1;
	if(pBrw && pData && pStyle) {
		const  BrowserDef * p_def = pBrw->getDef();
		if(col >= 0 && col < (long)p_def->getCount()) {
			const BroColumn & r_col = p_def->at(col);
			PPViewVetisDocument::BrwHdr * p_hdr = (PPViewVetisDocument::BrwHdr *)pData;
			if(r_col.OrgOffs == 14) { // waybillnumber
				if(p_hdr->LinkBillID) {
					if(p_hdr->LinkBillRow > 0) {
						pStyle->RightFigColor = GetColorRef(SClrGreen);
					}
					else {
						pStyle->RightFigColor = GetColorRef(SClrYellow);
					}
					pStyle->Flags |= pStyle->fRightFigCircle;
					ok = 1;
				}
			}
			if(r_col.OrgOffs == 15) { // goods
				if(p_hdr->LinkGoodsID) {
					//pStyle->Flags |= BrowserWindow::CellStyle::fCorner;
					pStyle->Color = GetColorRef(SClrLightgreen);
					ok = 1;
				}
			}
			else if(r_col.OrgOffs == 17) { // from
				if(p_hdr->Flags & VetisVetDocument::fFromMainOrg) {
					pStyle->Color = GetColorRef(SClrLightblue);
					ok = 1;
				}
				else if(p_hdr->LinkFromDlvrLocID) {
					pStyle->Color = GetColorRef(SClrLightgreen);
					ok = 1;
				}
				else if(p_hdr->LinkFromPsnID) {
					pStyle->Color = GetColorRef(SClrLightcyan);
					ok = 1;
				}
			}
			else if(r_col.OrgOffs == 18) { // to
				if(p_hdr->Flags & VetisVetDocument::fToMainOrg) {
					pStyle->Color = GetColorRef(SClrLightblue);
					ok = 1;
				}
				else if(p_hdr->LinkToDlvrLocID) {
					pStyle->Color = GetColorRef(SClrLightgreen);
					ok = 1;
				}
				else if(p_hdr->LinkToPsnID) {
					pStyle->Color = GetColorRef(SClrLightcyan);
					ok = 1;
				}
			}
		}
	}
	return ok;
}

void SLAPI PPViewVetisDocument::PreprocessBrowser(PPViewBrowser * pBrw)
{
	CALLPTRMEMB(pBrw, SetCellStyleFunc(CellStyleFunc, pBrw));
}

int SLAPI PPViewVetisDocument::ProcessIncoming(PPID entityID)
{
	int    ok = -1;
	VetisVetDocument item;
	if(EC.Get(entityID, item) > 0) {
		if(!item.Uuid.IsZero()) {
			PPVetisInterface::Param param(0, Filt.LocID);
			THROW(PPVetisInterface::SetupParam(param));
			{
				PPLogger logger;
				PPVetisInterface ifc(&logger);
				VetisApplicationBlock reply;
				THROW(ifc.Init(param));
				ifc.ProcessIncomingConsignment(item.Uuid, reply);
			}
		}
	}
	CATCHZOKPPERR
	return ok;
}

int SLAPI PPViewVetisDocument::LoadDocuments()
{
	int    ok = -1;
	uint   v = 0;
	PPVetisInterface::Param param(0, Filt.LocID);
	THROW(PPVetisInterface::SetupParam(param));
	if(SelectorDialog(DLG_SELVDOCUPD, STDCTL_SELECTOR_WHAT, &v) > 0) {		
		PPLogger logger;
		PPVetisInterface ifc(&logger);
		VetisApplicationBlock reply;
		TSVector <VetisEntityCore::UnresolvedEntity> ure_list;
		const uint req_count = 1000;
		THROW(ifc.Init(param));
		PPWait(1);
		if(v == 0) { // изменения
			STimeChunk tc;
			if(checkdate(Filt.Period.low)) {
				tc.Start.Set(Filt.Period.low, ZEROTIME);
				if(checkdate(Filt.Period.upp))
					tc.Finish.Set(Filt.Period.upp, encodetime(23, 59, 59, 99));
				else 
					tc.Finish.Set(Filt.Period.low, encodetime(23, 59, 59, 99));
			}
			else if(checkdate(Filt.Period.upp)) {
				tc.Start.Set(Filt.Period.upp, ZEROTIME);
				tc.Finish.Set(Filt.Period.upp, encodetime(23, 59, 59, 99));
			}
			else {
				const LDATE cd = getcurdate_();
				tc.Start.Set(plusdate(cd, -1), ZEROTIME);
				tc.Finish.Set(cd, encodetime(23, 59, 59, 99));
			}
			for(uint req_offs = 0; ifc.GetVetDocumentChangesList(tc, req_offs, req_count, reply);) {
				PPTransaction tra(1);
				THROW(tra);
				for(uint i = 0; i < reply.VetDocList.getCount(); i++) {
					const VetisVetDocument * p_item = reply.VetDocList.at(i);
					if(p_item) {
						PPID   pi_id = 0;
						THROW(ifc.PeC.Put(&pi_id, *p_item, &ure_list, 0));
						ok = 1;
					}
				}
				THROW(tra.Commit());
				PPWaitPercent(reply.ListResult.Count+reply.ListResult.Offset, reply.ListResult.Total);
				if(reply.VetDocList.getCount() < req_count)
					break;
				else
					req_offs += reply.VetDocList.getCount();
			}
		}
		else if(v == 1) { // полная загрузка
			for(uint req_offs = 0; ifc.GetVetDocumentList(req_offs, req_count, reply);) {
				PPTransaction tra(1);
				THROW(tra);
				for(uint i = 0; i < reply.VetDocList.getCount(); i++) {
					const VetisVetDocument * p_item = reply.VetDocList.at(i);
					if(p_item) {
						PPID   pi_id = 0;
						THROW(ifc.PeC.Put(&pi_id, *p_item, &ure_list, 0));
						ok = 1;
					}
				}
				THROW(tra.Commit());
				PPWaitPercent(reply.ListResult.Count+reply.ListResult.Offset, reply.ListResult.Total);
				if(reply.VetDocList.getCount() < req_count)
					break;
				else
					req_offs += reply.VetDocList.getCount();
			}
		}
		if(ok > 0)
			THROW(ifc.ProcessUnresolvedEntityList(ure_list));
	}
	CATCHZOKPPERR
	PPWait(0);
	return ok;
}

struct PPMatchPersonBlock {
	SLAPI  PPMatchPersonBlock() : PersonKindID(0), PersonID(0), DlvrLocID(0)
	{
	}
	SString SrcInfo;
	PPID   PersonKindID;
	PPID   PersonID;
	PPID   DlvrLocID;
};

class MatchPersonDialog : public TDialog {
public:
	MatchPersonDialog() : TDialog(DLG_MTCHPSN)
	{
	}
	int    setDTS(const PPMatchPersonBlock * pData)
	{
		int    ok = 1;
		RVALUEPTR(Data, pData);
		setCtrlString(CTL_MTCHPSN_SRCINFO, Data.SrcInfo);
		SetupPPObjCombo(this, CTLSEL_MTCHPSN_PSN, PPOBJ_PERSON, Data.PersonID, OLW_CANINSERT, (void *)Data.PersonKindID);
		SetupPPObjCombo(this, CTLSEL_MTCHPSN_LOC, PPOBJ_LOCATION, Data.DlvrLocID, OLW_CANINSERT, 0);
		selectCtrl(CTL_MTCHPSN_PSN);
		if(Data.PersonID) 
			PsnObj.SetupDlvrLocCombo(this, CTLSEL_MTCHPSN_LOC, Data.PersonID, Data.DlvrLocID);
		return ok;
	}
	int    getDTS(PPMatchPersonBlock * pData)
	{
		int    ok = 1;
		getCtrlData(CTLSEL_MTCHPSN_PSN, &Data.PersonID);
		getCtrlData(CTLSEL_MTCHPSN_LOC, &Data.DlvrLocID);
		ASSIGN_PTR(pData, Data);
		return ok;
	}
private:
	DECL_HANDLE_EVENT
	{
		TDialog::handleEvent(event);
		if(event.isCbSelected(CTLSEL_MTCHPSN_PSN)) {
			const PPID preserve_loc_id = Data.DlvrLocID;
			const PPID preserve_psn_id = Data.PersonID;
			getCtrlData(CTLSEL_MTCHPSN_PSN, &Data.PersonID);
			if(preserve_psn_id != Data.PersonID) {
				if(Data.PersonID) 
					PsnObj.SetupDlvrLocCombo(this, CTLSEL_MTCHPSN_LOC, Data.PersonID, Data.DlvrLocID);
			}
		}
		else
			return;
		clearEvent(event);
	}
	PPMatchPersonBlock Data;
	PPObjPerson PsnObj;
};

int SLAPI PPViewVetisDocument::MatchObject(VetisDocumentTbl::Rec & rRec, int objToMatch)
{
	int    ok = -1;
	PPObjAccSheet acs_obj;
	if(oneof2(objToMatch, otmFrom, otmTo)) {
		const int  side = (objToMatch == otmTo) ? 1 : 0;
		PPMatchPersonBlock mb;
		const PPID acs_id = (objToMatch == otmTo) ? GetSellAccSheet() : GetSupplAccSheet();
		PPAccSheet acs_rec;
		if(acs_obj.Fetch(acs_id, &acs_rec) > 0 && acs_rec.Assoc == PPOBJ_PERSON && acs_rec.ObjGroup)
			mb.PersonKindID = acs_rec.ObjGroup;
		else
			mb.PersonKindID = (objToMatch == otmTo) ? PPPRK_CLIENT : PPPRK_SUPPL;
		mb.PersonID = (objToMatch == otmTo) ? rRec.LinkToPsnID : rRec.LinkFromPsnID;
		mb.DlvrLocID = (objToMatch == otmTo) ? rRec.LinkToDlvrLocID : rRec.LinkFromDlvrLocID;
		int   r = PPDialogProcBody <MatchPersonDialog, PPMatchPersonBlock> (&mb);
		if(r > 0 && mb.PersonID) {
			ok = EC.MatchPersonInDocument(rRec.EntityID, side, mb.PersonID, mb.DlvrLocID, 1);
			if(!ok)
				PPError();
		}
	}
	else if(objToMatch == otmGoods) {
	}
	else if(objToMatch == otmBill) {
		if((rRec.LinkFromDlvrLocID || rRec.LinkFromPsnID) && (rRec.LinkToDlvrLocID || rRec.LinkToPsnID)) {
			PPObjArticle ar_obj;
			PPObjPerson psn_obj;
			int i_am_side = -1; // 0 - приход (главная организация TO), 1 - расход (главная организация FROM)
			if(rRec.Flags & VetisVetDocument::fToMainOrg) {
				i_am_side = 0;
			}
			else if(rRec.Flags & VetisVetDocument::fFromMainOrg) {
				i_am_side = 1;
			}
			if(i_am_side == 0) {
				PPID   acs_id = GetSupplAccSheet();
				PPID   ar_id = 0;
				PPID   loc_id = 0;
				PPID   bill_id = 0;
				if(rRec.LinkFromPsnID) {
					ar_obj.P_Tbl->PersonToArticle(rRec.LinkFromPsnID, acs_id, &ar_id);
				}
				if(rRec.LinkFromDlvrLocID) {
					LocationTbl::Rec loc_rec;
					if(psn_obj.LocObj.Fetch(rRec.LinkFromDlvrLocID, &loc_rec) > 0) {
						loc_id = loc_rec.ID;
						if(!ar_id && loc_rec.OwnerID)
							ar_obj.P_Tbl->PersonToArticle(loc_rec.OwnerID, acs_id, &ar_id);
					}
				}
				if(ar_id) {
					BillFilt bill_filt;
					//bill_filt.LocList.Add(loc_id);
					//bill_filt.OpID  = op_id;
					bill_filt.ObjectID = ar_id;
					bill_filt.Flags |= BillFilt::fAsSelector;
					if(checkdate(rRec.WayBillDate)) {
						bill_filt.Period.Set(rRec.WayBillDate, plusdate(rRec.WayBillDate, 2));
					}
					else if(checkdate(rRec.IssueDate)) {
						bill_filt.Period.Set(rRec.IssueDate, plusdate(rRec.WayBillDate, 5));
					}
					if(!bill_filt.Period.IsZero()) {
						PPViewBill bill_view;
						if(bill_view.Init_(&bill_filt)) {
							if(bill_view.Browse(0) > 0) {
								bill_id = ((BillFilt*)bill_view.GetBaseFilt())->Sel;
								ok = EC.MatchDocument(rRec.EntityID, bill_id, 0, 1);
								if(!ok)
									PPError();
							}
						}
					}
				}
			}
		}
	}
	return ok;
}

int SLAPI PPViewVetisDocument::ProcessCommand(uint ppvCmd, const void * pHdr, PPViewBrowser * pBrw)
{
	int    ok = PPView::ProcessCommand(ppvCmd, pHdr, pBrw);
	if(ok == -2) {
		PPID   id = pHdr ? *(PPID *)pHdr : 0;
		switch(ppvCmd) {
			case PPVCMD_EDITITEM:
				ok = -1;
				if(id) {
					VetisVetDocument item;
					if(EC.Get(id, item) > 0) {
						EditVetisVetDocument(item);
					}
				}
				break;
			case PPVCMD_UTILIZEDVDOC:
				ok = -1;
				if(id) {
					ok = ProcessIncoming(id);
				}
				break;
			case PPVCMD_UPDATEITEMS:
				ok = LoadDocuments();
				break;
			case PPVCMD_MATCHPERSON:
				ok = -1;
				if(id) {
					const  int ccol = pBrw->GetCurColumn();
					int    obj_to_match = 0;
					if(ccol == 6) { // from
						obj_to_match = otmFrom;
					}
					else if(ccol == 7) { // to
						obj_to_match = otmTo;
					}
					else if(oneof2(ccol, 4, 5)) { // bill
						obj_to_match = otmBill;
					}
					if(obj_to_match) {
						VetisDocumentTbl::Rec rec;
						if(EC.SearchDocument(id, &rec) > 0) {
							ok = MatchObject(rec, obj_to_match);
						}
					}
				}
				break;
		}
	}
	return ok;
}

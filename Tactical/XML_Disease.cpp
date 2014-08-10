#ifdef PRECOMPILEDHEADERS
#include "Tactical All.h"
#else
#include "sgp.h"
#include "overhead.h"
#include "Disease.h"
#include "Debug Control.h"
#include "expat.h"
#include "XML.h"
#endif

struct
{
	PARSE_STAGE	curElement;

	CHAR8			szCharData[MAX_CHAR_DATA_LENGTH + 1];
	DISEASE			curItem;
	DISEASE *		curArray;
	UINT32			maxArraySize;

	UINT32			currentDepth;
	UINT32			maxReadDepth;
}
typedef parseData;

static void XMLCALL
diseaseStartElementHandle( void *userData, const XML_Char *name, const XML_Char **atts )
{
	parseData * pData = (parseData *)userData;

	if ( pData->currentDepth <= pData->maxReadDepth ) //are we reading this element?
	{
		if ( strcmp( name, "DISEASESLIST" ) == 0 && pData->curElement == ELEMENT_NONE )
		{
			pData->curElement = ELEMENT_LIST;

			memset( pData->curArray, 0, sizeof(DISEASE)*pData->maxArraySize );

			pData->maxReadDepth++; //we are not skipping this element
		}
		else if ( strcmp( name, "DISEASE" ) == 0 && pData->curElement == ELEMENT_LIST )
		{
			pData->curElement = ELEMENT;

			memset( &pData->curItem, 0, sizeof(DISEASE) );

			pData->maxReadDepth++; //we are not skipping this element
		}
		else if ( pData->curElement == ELEMENT &&
				  (strcmp( name, "uiIndex" ) == 0 ||
				  strcmp( name, "szName" ) == 0 ||
				  strcmp( name, "szFatName" ) == 0 ||
				  strcmp( name, "szDescription" ) == 0 ||
				  strcmp( name, "sInfectionPtsInitial" ) == 0 ||
				  strcmp( name, "sInfectionPtsOutbreak" ) == 0 ||
				  strcmp( name, "sInfectionPtsFull" ) == 0 ||
				  strcmp( name, "sInfectionPtsGainPerHour" ) == 0 ||
				  strcmp( name, "usInfectionChance_SWAMP" ) == 0 ||
				  strcmp( name, "usInfectionChance_TROPICS" ) == 0 ||
				  strcmp( name, "usInfectionChance_SEX" ) == 0 ||
				  strcmp( name, "usInfectionChance_CONTACT_HUMAN" ) == 0 ||
				  strcmp( name, "usInfectionChance_CONTACT_CORPSE" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_ANIMAL" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_OPEN" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_GUNSHOT" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_AGI" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_DEX" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_STR" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_WIS" ) == 0 ||
				  strcmp( name, "usInfectionChance_WOUND_TRAUMATIC" ) == 0 ||
				  strcmp( name, "usInfectionChance_BADFOOD" ) == 0 ||
				  strcmp( name, "usInfectionChance_BADWATER" ) == 0 ||
				  strcmp( name, "fCanBeCured" ) == 0 ||
				  strcmp( name, "fReverseOnFull" ) == 0 ||
				  strcmp( name, "fCanReInfect" ) == 0 ||
				  strcmp( name, "fHideSymbol" ) == 0 ||
				  strcmp( name, "fDisgusting" ) == 0 ||
				  strcmp( name, "sEffStatAGI" ) == 0 ||
				  strcmp( name, "sEffStatDEX" ) == 0 ||
				  strcmp( name, "sEffStatSTR" ) == 0 ||
				  strcmp( name, "sEffStatWIS" ) == 0 ||
				  strcmp( name, "sEffStatEXP" ) == 0 ||
				  strcmp( name, "sEffAP" ) == 0 ||
				  strcmp( name, "usMaxBreath" ) == 0 ||
				  strcmp( name, "sEffCarryStrength" ) == 0 ||
				  strcmp( name, "sLifeRegenHundreds" ) == 0 ||
				  strcmp( name, "sNeedToSleep" ) == 0 ||
				  strcmp( name, "sDrinkModifier" ) == 0 ||
				  strcmp( name, "sFoodModifier" ) == 0) )
		{
			pData->curElement = ELEMENT_PROPERTY;

			pData->maxReadDepth++; //we are not skipping this element
		}

		pData->szCharData[0] = '\0';
	}

	pData->currentDepth++;

}

static void XMLCALL
diseaseCharacterDataHandle( void *userData, const XML_Char *str, int len )
{
	parseData * pData = (parseData *)userData;

	if ( (pData->currentDepth <= pData->maxReadDepth) &&
		 (strlen( pData->szCharData ) < MAX_CHAR_DATA_LENGTH)
		 ){
		strncat( pData->szCharData, str, __min( (unsigned int)len, MAX_CHAR_DATA_LENGTH - strlen( pData->szCharData ) ) );
	}
}


static void XMLCALL
diseaseEndElementHandle( void *userData, const XML_Char *name )
{
	parseData * pData = (parseData *)userData;

	if ( pData->currentDepth <= pData->maxReadDepth ) //we're at the end of an element that we've been reading
	{
		if ( strcmp( name, "DISEASESLIST" ) == 0 )
		{
			pData->curElement = ELEMENT_NONE;
		}
		else if ( strcmp( name, "DISEASE" ) == 0 )
		{
			pData->curElement = ELEMENT_LIST;

			// we do NOT want to read the first entry -> move stuff by 1
			if ( pData->curItem.uiIndex < pData->maxArraySize )
			{
				pData->curArray[pData->curItem.uiIndex] = pData->curItem; //write the food into the table
			}
		}
		else if ( strcmp( name, "uiIndex" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.uiIndex = (UINT16)atol( pData->szCharData );
		}
		else if ( strcmp( name, "szName" ) == 0 )
		{
			pData->curElement = ELEMENT;

			MultiByteToWideChar( CP_UTF8, 0, pData->szCharData, -1, pData->curItem.szName, sizeof(pData->curItem.szName) / sizeof(pData->curItem.szName[0]) );
			pData->curItem.szName[sizeof(pData->curItem.szName) / sizeof(pData->curItem.szName[0]) - 1] = '\0';
		}
		else if ( strcmp( name, "szFatName" ) == 0 )
		{
			pData->curElement = ELEMENT;

			MultiByteToWideChar( CP_UTF8, 0, pData->szCharData, -1, pData->curItem.szFatName, sizeof(pData->curItem.szFatName) / sizeof(pData->curItem.szFatName[0]) );
			pData->curItem.szFatName[sizeof(pData->curItem.szFatName) / sizeof(pData->curItem.szFatName[0]) - 1] = '\0';
		}
		else if ( strcmp( name, "szDescription" ) == 0 )
		{
			pData->curElement = ELEMENT;

			MultiByteToWideChar( CP_UTF8, 0, pData->szCharData, -1, pData->curItem.szDescription, sizeof(pData->curItem.szDescription) / sizeof(pData->curItem.szDescription[0]) );
			pData->curItem.szDescription[sizeof(pData->curItem.szDescription) / sizeof(pData->curItem.szDescription[0]) - 1] = '\0';
		}
		else if ( strcmp( name, "sInfectionPtsInitial" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sInfectionPtsInitial = (INT32)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sInfectionPtsOutbreak" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sInfectionPtsOutbreak = (INT32)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sInfectionPtsFull" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sInfectionPtsFull = (INT32)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sInfectionPtsGainPerHour" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sInfectionPtsGainPerHour = (INT32)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_SWAMP" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_SWAMP] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_TROPICS" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_TROPICS] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_SEX" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_SEX] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_CONTACT_HUMAN" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_CONTACT_HUMAN] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_CONTACT_CORPSE" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_CONTACT_CORPSE] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_ANIMAL" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_WOUND_ANIMAL] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_OPEN" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_WOUND_OPEN] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_GUNSHOT" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_WOUND_GUNSHOT] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_AGI" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_WOUND_AGI] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_DEX" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_WOUND_DEX] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_STR" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_WOUND_STR] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_WIS" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_WOUND_WIS] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_WOUND_TRAUMATIC" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_TRAUMATIC] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_BADFOOD" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_BADFOOD] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usInfectionChance_BADWATER" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usInfectionChance[INFECTION_TYPE_BADWATER] = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "fCanBeCured" ) == 0 )
		{
			pData->curElement = ELEMENT;
			if ( atol( pData->szCharData ) )
				 pData->curItem.usDiseaseProperties |= DISEASE_PROPERTY_CANBECURED;
		}
		else if ( strcmp( name, "fReverseOnFull" ) == 0 )
		{
			pData->curElement = ELEMENT;
			if ( atol( pData->szCharData ) )
				pData->curItem.usDiseaseProperties |= DISEASE_PROPERTY_REVERSEONFULL;
		}
		else if ( strcmp( name, "fCanReInfect" ) == 0 )
		{
			pData->curElement = ELEMENT;
			if ( atol( pData->szCharData ) )
				pData->curItem.usDiseaseProperties |= DISEASE_PROPERTY_CANREINFECT;
		}
		else if ( strcmp( name, "fHideSymbol" ) == 0 )
		{
			pData->curElement = ELEMENT;
			if ( atol( pData->szCharData ) )
				pData->curItem.usDiseaseProperties |= DISEASE_PROPERTY_HIDESYMBOL;
		}
		else if ( strcmp( name, "fDisgusting" ) == 0 )
		{
			pData->curElement = ELEMENT;
			if ( atol( pData->szCharData ) )
				pData->curItem.usDiseaseProperties |= DISEASE_PROPERTY_DISGUSTING;
		}
		else if ( strcmp( name, "sEffStatAGI" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sEffStat[INFST_AGI] = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sEffStatDEX" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sEffStat[INFST_DEX] = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sEffStatSTR" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sEffStat[INFST_STR] = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sEffStatWIS" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sEffStat[INFST_WIS] = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sEffStatEXP" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sEffStat[INFST_EXP] = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sEffAP" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sEffAP = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "usMaxBreath" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.usMaxBreath = (UINT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sEffCarryStrength" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sEffCarryStrength = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sLifeRegenHundreds" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sLifeRegenHundreds = (INT16)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sNeedToSleep" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sNeedToSleep = (INT8)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sDrinkModifier" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sDrinkModifier = (INT16)atol( pData->szCharData );
		}
		else if ( strcmp( name, "sFoodModifier" ) == 0 )
		{
			pData->curElement = ELEMENT;
			pData->curItem.sFoodModifier = (INT16)atol( pData->szCharData );
		}

		pData->maxReadDepth--;
	}

	pData->currentDepth--;
}


BOOLEAN ReadInDiseaseStats( STR fileName )
{
	HWFILE		hFile;
	UINT32		uiBytesRead;
	UINT32		uiFSize;
	CHAR8 *		lpcBuffer;
	XML_Parser	parser = XML_ParserCreate( NULL );

	parseData pData;

	DebugMsg( TOPIC_JA2, DBG_LEVEL_3, "Loading Disease.xml" );

	// Open foods file
	hFile = FileOpen( fileName, FILE_ACCESS_READ, FALSE );
	if ( !hFile )
		return(FALSE);

	uiFSize = FileGetSize( hFile );
	lpcBuffer = (CHAR8 *)MemAlloc( uiFSize + 1 );

	//Read in block
	if ( !FileRead( hFile, lpcBuffer, uiFSize, &uiBytesRead ) )
	{
		MemFree( lpcBuffer );
		return(FALSE);
	}

	lpcBuffer[uiFSize] = 0; //add a null terminator

	FileClose( hFile );

	XML_SetElementHandler( parser, diseaseStartElementHandle, diseaseEndElementHandle );
	XML_SetCharacterDataHandler( parser, diseaseCharacterDataHandle );

	memset( &pData, 0, sizeof(pData) );
	pData.curArray = Disease;
	pData.maxArraySize = NUM_DISEASES;

	XML_SetUserData( parser, &pData );

	if ( !XML_Parse( parser, lpcBuffer, uiFSize, TRUE ) )
	{
		CHAR8 errorBuf[511];

		sprintf( errorBuf, "XML Parser Error in Disease.xml: %s at line %d", XML_ErrorString( XML_GetErrorCode( parser ) ), XML_GetCurrentLineNumber( parser ) );
		LiveMessage( errorBuf );

		MemFree( lpcBuffer );
		return FALSE;
	}

	MemFree( lpcBuffer );

	XML_ParserFree( parser );

	return(TRUE);
}

BOOLEAN WriteDiseaseStats( )
{
	//DebugMsg (TOPIC_JA2,DBG_LEVEL_3,"writefoodsstats");
	HWFILE		hFile;

	//Debug code; make sure that what we got from the file is the same as what's there
	// Open a new file
	hFile = FileOpen( "TABLEDATA\\Disease out.xml", FILE_ACCESS_WRITE | FILE_CREATE_ALWAYS, FALSE );
	if ( !hFile )
		return(FALSE);

	{
		FilePrintf( hFile, "<DISEASESLIST>\r\n" );
		for ( UINT32 cnt = 0; cnt < NUM_DISEASES; ++cnt )
		{
			FilePrintf( hFile, "\t<DISEASE>\r\n" );

			FilePrintf( hFile, "\t\t<uiIndex>%d</uiIndex>\r\n", cnt );
			FilePrintf( hFile, "\t\t<szName>%s</szName>\r\n", Disease[cnt].szName );
			FilePrintf( hFile, "\t\t<szFatName>%s</szFatName>\r\n", Disease[cnt].szFatName );
			FilePrintf( hFile, "\t\t<szDescription>%s</szDescription>\r\n", Disease[cnt].szDescription );
			FilePrintf( hFile, "\t\t<sInfectionPtsInitial>%d</sInfectionPtsInitial>\r\n", Disease[cnt].sInfectionPtsInitial );
			FilePrintf( hFile, "\t\t<sInfectionPtsOutbreak>%d</sInfectionPtsOutbreak>\r\n", Disease[cnt].sInfectionPtsOutbreak );
			FilePrintf( hFile, "\t\t<sInfectionPtsFull>%d</sInfectionPtsFull>\r\n", Disease[cnt].sInfectionPtsFull );
			FilePrintf( hFile, "\t\t<sInfectionPtsGainPerHour>%d</sInfectionPtsGainPerHour>\r\n", Disease[cnt].sInfectionPtsGainPerHour );
			FilePrintf( hFile, "\t\t<usInfectionChance_SWAMP>%d</usInfectionChance_SWAMP>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_SWAMP] );
			FilePrintf( hFile, "\t\t<usInfectionChance_TROPICS>%d</usInfectionChance_TROPICS>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_TROPICS] );
			FilePrintf( hFile, "\t\t<usInfectionChance_SEX>%d</usInfectionChance_SEX>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_SEX] );
			FilePrintf( hFile, "\t\t<usInfectionChance_CONTACT_HUMAN>%d</usInfectionChance_CONTACT_HUMAN>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_CONTACT_HUMAN] );
			FilePrintf( hFile, "\t\t<usInfectionChance_CONTACT_CORPSE>%d</usInfectionChance_CONTACT_CORPSE>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_CONTACT_CORPSE] );
			FilePrintf( hFile, "\t\t<usInfectionChance_WOUND_ANIMAL>%d</usInfectionChance_WOUND_ANIMAL>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_WOUND_ANIMAL] );
			FilePrintf( hFile, "\t\t<usInfectionChance_WOUND_OPEN>%d</usInfectionChance_WOUND_OPEN>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_WOUND_OPEN] );
			FilePrintf( hFile, "\t\t<usInfectionChance_WOUND_GUNSHOT>%d</usInfectionChance_WOUND_GUNSHOT>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_WOUND_GUNSHOT] );
			FilePrintf( hFile, "\t\t<usInfectionChance_WOUND_AGI>%d</usInfectionChance_WOUND_AGI>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_WOUND_AGI] );
			FilePrintf( hFile, "\t\t<usInfectionChance_WOUND_DEX>%d</usInfectionChance_WOUND_DEX>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_WOUND_DEX] );
			FilePrintf( hFile, "\t\t<usInfectionChance_WOUND_STR>%d</usInfectionChance_WOUND_STR>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_WOUND_STR] );
			FilePrintf( hFile, "\t\t<usInfectionChance_WOUND_WIS>%d</usInfectionChance_WOUND_WIS>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_WOUND_WIS] );			
			FilePrintf( hFile, "\t\t<usInfectionChance_TRAUMATIC>%d</usInfectionChance_TRAUMATIC>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_TRAUMATIC] );
			FilePrintf( hFile, "\t\t<usInfectionChance_BADFOOD>%d</usInfectionChance_BADFOOD>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_BADFOOD] );
			FilePrintf( hFile, "\t\t<usInfectionChance_BADWATER>%d</usInfectionChance_BADWATER>\r\n", Disease[cnt].usInfectionChance[INFECTION_TYPE_BADWATER] );
			FilePrintf( hFile, "\t\t<fCanBeCured>%d</fCanBeCured>\r\n", (Disease[cnt].usDiseaseProperties & DISEASE_PROPERTY_CANBECURED) ? 1 : 0 );
			FilePrintf( hFile, "\t\t<fReverseOnFull>%d</fReverseOnFull>\r\n", (Disease[cnt].usDiseaseProperties & DISEASE_PROPERTY_REVERSEONFULL) ? 1 : 0 );
			FilePrintf( hFile, "\t\t<fCanReInfect>%d</fCanReInfect>\r\n", (Disease[cnt].usDiseaseProperties & DISEASE_PROPERTY_CANREINFECT) ? 1 : 0 );
			FilePrintf( hFile, "\t\t<fHideSymbol>%d</fHideSymbol>\r\n", (Disease[cnt].usDiseaseProperties & DISEASE_PROPERTY_HIDESYMBOL) ? 1 : 0 );
			FilePrintf( hFile, "\t\t<fDisgusting>%d</fDisgusting>\r\n", (Disease[cnt].usDiseaseProperties & DISEASE_PROPERTY_DISGUSTING) ? 1 : 0 );
			FilePrintf( hFile, "\t\t<sEffStatAGI>%d</sEffStatAGI>\r\n", Disease[cnt].sEffStat[INFST_AGI] );
			FilePrintf( hFile, "\t\t<sEffStatDEX>%d</sEffStatDEX>\r\n", Disease[cnt].sEffStat[INFST_DEX] );
			FilePrintf( hFile, "\t\t<sEffStatSTR>%d</sEffStatSTR>\r\n", Disease[cnt].sEffStat[INFST_STR] );
			FilePrintf( hFile, "\t\t<sEffStatWIS>%d</sEffStatWIS>\r\n", Disease[cnt].sEffStat[INFST_WIS] );
			FilePrintf( hFile, "\t\t<sEffStatEXP>%d</sEffStatEXP>\r\n", Disease[cnt].sEffStat[INFST_EXP] );
			FilePrintf( hFile, "\t\t<sEffAP>%d</sEffAP>\r\n", Disease[cnt].sEffAP );
			FilePrintf( hFile, "\t\t<usMaxBreath>%d</usMaxBreath>\r\n", Disease[cnt].usMaxBreath );
			FilePrintf( hFile, "\t\t<sEffCarryStrength>%d</sEffCarryStrength>\r\n", Disease[cnt].sEffCarryStrength );
			FilePrintf( hFile, "\t\t<sLifeRegenHundreds>%d</sLifeRegenHundreds>\r\n", Disease[cnt].sLifeRegenHundreds );
			FilePrintf( hFile, "\t\t<sNeedToSleep>%d</sNeedToSleep>\r\n", Disease[cnt].sNeedToSleep );
			FilePrintf( hFile, "\t\t<sDrinkModifier>%d</sDrinkModifier>\r\n", Disease[cnt].sDrinkModifier );
			FilePrintf( hFile, "\t\t<sFoodModifier>%d</sFoodModifier>\r\n", Disease[cnt].sFoodModifier );

			FilePrintf( hFile, "\t</DISEASE>\r\n" );
		}
		FilePrintf( hFile, "</DISEASESLIST>\r\n" );
	}
	FileClose( hFile );

	return(TRUE);
}

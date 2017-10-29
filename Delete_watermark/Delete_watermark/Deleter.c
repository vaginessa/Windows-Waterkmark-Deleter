/*
*	Programa que ejecutar al arranque de Windows para establecer a 1 el 
*	check en los registros de windows que hacen que windows establezca 
*	la marca de agua.
*/

#ifndef UNICODE
	#define UNICODE
#endif

#ifndef _UNICODE
	#define _UNICODE
#endif

#include <windows.h> 
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>


#define REGISTRY HKEY_LOCAL_MACHINE
#define REGISTRY_PATH _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SoftwareProtectionPlatform\\Activation")
#define TSIZE sizeof(TCHAR)


BOOL ModifyRegistry();
BOOL DisplayPair(LPTSTR, DWORD, LPBYTE, DWORD);


int _tmain()
{
	ModifyRegistry();
	getchar();
	return 0;
}

BOOL ModifyRegistry()
/*
	Enumera y modifica el registro
*/
{
	HKEY hKeyOpened;
	DWORD retorno;
	// info para las subclaves
	DWORD numSubKeys, maxSubKeyLen, numValues, maxValueNameLen, maxValueLen;
	DWORD valueNameLen, valueLen, valueType;
	FILETIME lastWriteTime;
	LPTSTR subKeyName, valueName;
	LPBYTE value;
	DWORD index;

	// para la modificación
	BYTE modifyRegistry = 0;
	BYTE newValue = 0x1;

	// abrimos el handle de la clave
	retorno = RegOpenKeyEx(
		REGISTRY, //clave a abrir
		REGISTRY_PATH, // PATH hacia el registro a modificar
		0,
		KEY_ALL_ACCESS, // queremos todo el acceso
		&hKeyOpened
	);

	
	if (retorno != ERROR_SUCCESS)
	{
		_tprintf(_T("\n[-] NOOOOOOOOOOOO, ERROR OPENING \"HKEY_LOCAL_MACHINE\\%s\", error code: %d"), REGISTRY_PATH, GetLastError());
		exit(-1);
	}
	
	retorno = RegQueryInfoKey(
		hKeyOpened, // registro abierto
		NULL,
		NULL,
		NULL,
		&numSubKeys, // número de subclaves contenidas por la clave dada
		&maxSubKeyLen, // longitud de la subclave cuyo nombre tiene mayor longitud
		NULL,
		&numValues, // número de valores asociados a esta clave
		&maxValueNameLen, // longitud del nombre de valor más grande
		&maxValueLen, // tamaño del dato más grande
		NULL,
		&lastWriteTime // ultima fecha de escritura de la clave
	);
	
	if (retorno != ERROR_SUCCESS)
	{
		_tprintf(_T("\n[-] NOOOOOOOOOOOO, ERROR QUERING INFORMATION ABOUT \"HKEY_LOCAL_MACHINE\\%s\", error code: %d"), REGISTRY_PATH, GetLastError());
		exit(-1);
	}
	
	subKeyName = (LPTSTR)malloc(TSIZE * (maxSubKeyLen + 1));
	valueName = (LPTSTR)malloc(TSIZE * (maxValueNameLen + 1));
	value = (LPBYTE)malloc(maxValueLen);

	_tprintf(_T("Values of \"HKEY_LOCAL_MACHINE\\%s\""), REGISTRY_PATH);
	for (index = 0; index < numValues; index++)
	{
		valueNameLen = maxValueNameLen + 1;
		valueLen = maxValueLen + 1;

		retorno = RegEnumValue(
			hKeyOpened, // valor del hkey abierto anteriormente
			index,  // indice de clave-valor, irá aumentando
			valueName, // nombre del valor (memoria reservada anteriormente)
			&valueNameLen, // puntero al tamaño del nombre, será el máximo más 1, tamaño ValueName
			NULL,
			&valueType, // tipo del valor
			value, //puntero al valor gracias a malloc tendrá el tamaño suficiente
			&valueLen // tamaño del valor, tendrá el mayor tamaño + 1
		);

		if (retorno == ERROR_SUCCESS && GetLastError() == 0) 
		{
			DisplayPair(valueName, valueType, value, valueLen);
		}

		// modificar el valor
		if (_tcscmp(valueName, _T("Manual")) == 0)
		{
			if ( (DWORD)(*value) == 0) 
			{
				
				retorno = RegSetValueEx(
					hKeyOpened, // hKey de la clave a modificar
					valueName, // valor a modificar
					NULL,
					valueType, // tipo del valor a modificar
					&newValue, // nuevo valor
					sizeof(newValue) // tamaño del valor
				);

				if (retorno != ERROR_SUCCESS)
				{
					_tprintf(_T("\n[-] NOOOOOOOOOOOO, ERROR MODIFYING \"HKEY_LOCAL_MACHINE\\%s\", error code: %d"), REGISTRY_PATH, GetLastError());
					exit(-1);
				}
				else
				{
					modifyRegistry = 1;
				}
			}
			else if ((DWORD)(*value) == 1)
			{
				modifyRegistry = 2;
			}
		}

	}

	if (modifyRegistry == 1)
	{
		_tprintf(_T("\nSUCCESS"));
		_tprintf(_T("\nValue of Manual modified"));
	}
	else if (modifyRegistry == 2)
	{
		_tprintf(_T("\nVALUE ALREADY MODIFIED"));
	}
	
	free(subKeyName); free(valueName); free(value);
	RegCloseKey(hKeyOpened);
	return TRUE;

}

BOOL DisplayPair(LPTSTR valueName, DWORD valueType, LPBYTE value, DWORD valueLen)
/*
	Método para mostrar la pareja clave-valor al usuario
*/
{
	LPBYTE pV = value;
	DWORD i;

	if (_tcscmp(valueName, _T("")) == 0)
	{
		if (_tcslen(_T("(Default)")) < valueLen)
		{
			_tcscpy_s(valueName, _tcslen(_T("(Default)")), _T("(Default)") );
		}
	}

	// mostramos el nombre (clave) de la pareja
	_tprintf(_T("\n%s = "), valueName);
	switch (valueType)
	{
	case REG_FULL_RESOURCE_DESCRIPTOR: // 9: Descriptor de hardware
	case REG_BINARY: // 3: Dato binario de cualquier forma
		for (i = 0; i < valueLen; i++, pV++)
		{
			_tprintf(_T(" %x"), (*pV));
		}
		break;
	case REG_DWORD: // 4: Un número de 32 bits
		_tprintf(_T(" %x"), (DWORD)(*value));
		break;
	case REG_EXPAND_SZ: // 2: cadena acabada en NULL no expandible
	case REG_MULTI_SZ: // 7: array de cadenas acabadas en NULL
	case REG_SZ: // 1: una cadena acabada en NULL
		_tprintf(_T(" %s"), (LPTSTR)value);
		break;
	}

	return TRUE;
}
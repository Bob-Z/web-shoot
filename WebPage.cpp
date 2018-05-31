/*
   Web-shooter is a shoot them up game with random graphics.
   Copyright (C) 2018 carabobz@gmail.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "WebPage.h"
#include "debug.h"
#include <string.h>

/*****************************************************************************/
WebPage::WebPage(const char * p_Url) :
	m_Url(p_Url)
	,m_Page()
	,m_Link()
	,m_Image()
	,m_NextPage(nullptr)
{
	printd(DEBUG_URL,"New page : %s\n",m_Url.c_str());
	if( web_to_memory( m_Url.c_str(), &m_Page) != -1)
	{
		parseLink();
		parseImage();
	}
}

/*****************************************************************************/
char * WebPage::getImageUrl()
{
	char * l_Ret = nullptr;

	if( m_Image.size() > 0 )
	{
		int l_Index = rand() % m_Image.size();
		l_Ret = strdup(m_Image[l_Index].c_str());

		m_Image.erase(m_Image.begin() + l_Index);

		printd(DEBUG_URL,"get %s in %s\n",l_Ret,m_Url.c_str());
		printd(DEBUG_URL,"%d Images left in %s\n",m_Image.size(),m_Url.c_str());

		return l_Ret;
	}

	if( m_NextPage != nullptr )
	{
		l_Ret = m_NextPage->getImageUrl();
		if( l_Ret != nullptr )
		{
			return l_Ret;
		}

		delete(m_NextPage);
		m_NextPage = nullptr;
	}


	while(m_Link.size() > 0)
	{
		int l_Index = rand() % m_Link.size();

		m_NextPage = new WebPage(m_Link[l_Index].c_str());
		m_Link.erase(m_Link.begin() + l_Index);
		printd(DEBUG_URL,"%d Links left in %s\n",m_Link.size(),m_Url.c_str());

		l_Ret = m_NextPage->getImageUrl();

		if( l_Ret != nullptr )
		{
			return l_Ret;
		}

		delete(m_NextPage);
		m_NextPage = nullptr;
	}

	
	printd(DEBUG_URL,"%s is empty\n",m_Url.c_str());
	return nullptr;
}

/*****************************************************************************/
void WebPage::parseLink()
{
        char * substring = NULL;
        char * substring_start = NULL;
        char * substring_end = NULL;

        int read_index = 0;

        // try to find an image's URL
        while(true)
        {
                substring=strstr(&m_Page.data[read_index],"href=\"");

                if( substring == NULL ) {
                        break;
                }

                // get the url
                substring_start = substring + sizeof("href=\"") - 1;
                substring_end = strstr(substring_start,"\"");

		char * l_Url = strndup(substring_start,substring_end-substring_start);

		m_Link.push_back(getValidUrl(l_Url));

		free(l_Url);

                read_index = substring_end - m_Page.data;
        }

        printd(DEBUG_URL,"%d links in URL %s\n",m_Link.size(), m_Url.c_str());
}

/*****************************************************************************/
void WebPage::parseImage()
{
        char * substring = NULL; 
        char * substring_start = NULL;
        char * substring_end = NULL;
	char * url = NULL;

	int read_index = 0;

	while(true)
	{
		substring=strstr(&m_Page.data[read_index],"<img ");
		if( substring == NULL ) {
			break;
		}

		read_index = substring - m_Page.data + strlen("<img");

		substring=strstr(&m_Page.data[read_index],"src=");
		if( substring == NULL ) {
			break;
		}

		// get the url
		substring_start = substring + strlen("src=") +1 ; // +1 is for " or ' after =
		char substring_final_char[2];
		substring_final_char[0] = *(substring_start - 1);
		substring_final_char[1] = 0;
		substring_end = strstr(substring_start,substring_final_char);

		char * partial_url = strndup(substring_start,substring_end-substring_start);

		m_Image.push_back(std::string(getValidUrl(partial_url)));

		free(partial_url);

		read_index = substring_end - m_Page.data;
	}

	printd(DEBUG_URL,"%d images in URL: %s\n",m_Image.size(), m_Url.c_str());
}

/*****************************************************************************/
std::string WebPage::getValidUrl(char * p_pUrl)
{
	char full_url[100000];

	if( strncmp(p_pUrl,"http",strlen("http")) == 0 )
	{
		strcpy(full_url,p_pUrl);
	}
	else
	{
		strcpy(full_url,m_Url.c_str());
		strcat(full_url,p_pUrl);
	}

	return std::string(full_url);
}

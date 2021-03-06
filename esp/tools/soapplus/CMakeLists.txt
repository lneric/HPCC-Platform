################################################################################
#    Copyright (C) 2011 HPCC Systems.
#
#    All rights reserved. This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License as
#    published by the Free Software Foundation, either version 3 of the
#    License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

# Component: soapplus 

#####################################################
# Description:
# ------------
#    Cmake Input File for soapplus
#####################################################


project( soapplus ) 

set (    SRCS 
         EspLogDeserializer.cpp 
         http.cpp 
         httpproxy.cpp 
         main.cpp 
         msggenerator.cpp 
         xmldiff.cpp 
    )

include_directories ( 
         ./../../../system/include 
         ./../../../system/security/securesocket 
         ./../../bindings/http/platform 
         ./../../../system/xmllib 
         ./../../../system/jlib 
    )

ADD_DEFINITIONS( -D_CONSOLE )

add_executable ( soapplus ${SRCS} )
install ( TARGETS soapplus DESTINATION ${OSSDIR}/bin )

target_link_libraries ( soapplus 
         jlib
         xmllib 
         securesocket 
    )



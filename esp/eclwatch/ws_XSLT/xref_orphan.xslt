<?xml version="1.0" encoding="UTF-8"?>
<!--

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <xsl:template match="Orphans">
        <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
          <head>
            <title>XRef - Orphaned Files</title>
               <link type="text/css" rel="StyleSheet" href="/esp/files_/css/sortabletable.css"/>
        <script type="text/javascript" src="/esp/files/scripts/espdefault.js">&#160;</script>
        <script type="text/javascript" src="/esp/files_/scripts/sortabletable.js">
                  <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
               </script>
               <script language="javascript" src="/esp/files_/scripts/multiselect.js">
                  <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
               </script>
        <link rel="stylesheet" type="text/css" href="/esp/files/yui/build/fonts/fonts-min.css" />
        <link rel="stylesheet" type="text/css" href="/esp/files/css/espdefault.css" />
        <script language="JavaScript1.2"><xsl:text disable-output-escaping="yes"><![CDATA[
               var sortableTable = null;
               function onRowCheck(checked)
               {
                  document.forms[0].deleteBtn.disabled = checkedCount == 0;
               }              
               
                function onLoad()
                {
                  initSelection('resultsTable');
                  onRowCheck(false);
                  var table = document.getElementById('resultsTable');
                  if (table)
                     sortableTable = new SortableTable(table, table, ["None", "String", "DateTime", "Number", "Number", "Number"]);
                }   

              function doconfirm(msg) {
                if (confirm('Delete selected Files?')) {
                  document.getElementById('listitems').style.display = 'none';
                  document.getElementById('DeletingHdr').style.display = 'block';
                  return true;
                }
                return false;
              }
                 ]]></xsl:text>
            </script>
          </head>
      <body class="yui-skin-sam" onload="nof5();onLoad()">
          <h3>Orphan files on '<xsl:value-of select="Cluster"/>' cluster:</h3>
        <h4 id="DeletingHdr" style="display:none;">Deleting Orphan files...</h4>
            <form id="listitems" action="/WsDFUXRef/DFUXRefArrayAction?Cluster={Cluster}&amp;Type=Orphan" method="post">
                <table class="sort-table" id="resultsTable">
                    <colgroup>
                       <col width="5"  class="number"/>
                       <col width="400"/>
                       <col width="400" class="date"/>
                       <col width="100" class="number"/>
                       <col width="100" class="number"/>
                       <col width="200" class="number"/>                       
                    </colgroup>
                    <thead>
                    <tr class="grey">
                        <th>
                                 <!--xsl:if test="File[2]">
                                    <xsl:attribute name="id">selectAll1</xsl:attribute>
                                    <input type="checkbox" title="Select or deselect all files" onclick="selectAll(this.checked)"/>
                                 </xsl:if-->
                        </th>
                        <th>Name</th> 
                        <th>Modified</th>
                        <th>Parts Found</th>
                        <th>Total Parts</th>
                        <th>Size</th>
                    </tr>
                         </thead>
                         <tbody>
                    <xsl:apply-templates select="File">
                        <xsl:sort select="Modified" data-type="number"/>
                    </xsl:apply-templates>
                        </tbody>
                </table>
                     <!--xsl:if test="File[2]">
                        <table  class="select-all">
                           <tr>
                              <th id="selectAll2">
                                 <input type="checkbox" title="Select or deselect all files" onclick="selectAll(this.checked)"/>
                              </th>
                              <th align="left" colspan="6">Select All / None</th>
                           </tr>
                        </table>
                     </xsl:if-->
                     <br/><br/>
                  <input id="deleteBtn" type="submit" class="sbutton" name="Action" value="Delete" disabled="true" onclick="return doconfirm('Delete selected Files?')"/>
            </form>
          </body> 
        </html>
    </xsl:template>
    
    <xsl:template match="File">
      <tr onmouseenter="this.bgColor = '#F0F0FF'">
         <xsl:choose>
            <xsl:when test="position() mod 2">
               <xsl:attribute name="bgColor">#FFFFFF</xsl:attribute>
               <xsl:attribute name="onmouseleave">this.bgColor = '#FFFFFF'</xsl:attribute>
            </xsl:when>
            <xsl:otherwise>
               <xsl:attribute name="bgColor">#F0F0F0</xsl:attribute>
               <xsl:attribute name="onmouseleave">this.bgColor = '#F0F0F0'</xsl:attribute>
            </xsl:otherwise>
         </xsl:choose>   
        <td><input type="checkbox" name="XRefFiles_i{position()}" value="{Partmask}" onclick="clicked(this, event)"/></td>

        <td align="left"><xsl:value-of select="Partmask"/> </td>
        <td><xsl:value-of select="Modified"/></td>
        <td><xsl:value-of select="Partsfound"/></td>
        <td><xsl:value-of select="Numparts"/></td>
        <td><xsl:value-of select="Size"/></td>
        </tr>
        <!--xsl:apply-templates select="Part">
                <xsl:sort select="Num" data-type="number" />
        </xsl:apply-templates-->
        
    </xsl:template>
    <xsl:template match="Part">
        <tr>
        <td></td>
        <td></td>
        <td></td>
        <td></td>
        <td></td>
        <td><xsl:value-of select="Node"/> </td>
        <td><xsl:value-of select="Num"/></td>
        </tr>
    </xsl:template>
</xsl:stylesheet>

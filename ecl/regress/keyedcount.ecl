/*##############################################################################

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
############################################################################## */

#option ('optimizeIndexSource', true);

childRecord :=
            record
unsigned6       id;
            end;

namesRecord :=
            RECORD
string20        surname;
string10        forename;
integer2        age := 25;
dataset(childRecord) ids;
unsigned8       filepos{virtual(fileposition)};
            END;

namesTable := dataset('x',namesRecord,FLAT);

i := index(namesTable, { surname, forename, age }, { dataset ids := ids, filepos }, 'i');


set of string20 surnames := [] : stored('surnames');

output(count(i(keyed(surname in surnames), keyed(forename <> 'Gavin', opt))));
output(count(i(keyed(surname in surnames), keyed(forename <> 'Gavin', opt)), keyed));
output(count(i(keyed(surname = 'Hawthorn'), age / 10 = 5)));
output(count(i(keyed(surname = 'Hawthorn'), age / 10 = 5), keyed));

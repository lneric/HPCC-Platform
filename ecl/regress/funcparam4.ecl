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

#option ('pickBestEngine', false);

namesRecord :=
            RECORD
string20        surname;
string10        forename;
integer2        age := 25;
            END;


datasetActionPrototype(dataset(namesRecord) in) := in;
noAction(dataset(namesRecord) in) := in;
limitAction(dataset(namesRecord) in) := limit(in, 500);
limitSkipAction(dataset(namesRecord) in) := limit(in, 500, skip);
sortAction(dataset(namesRecord) in) := sort(in, surname);

combineAction(dataset(namesRecord) in, datasetActionPrototype act1, datasetActionPrototype act2) := act2(act1(in));

limitAndSortAction(dataset(namesRecord) in) := combineAction(in, limitAction, sortAction);

processDs(dataset(namesRecord) in, datasetActionPrototype actionFunc = noAction) := function

x := in(age != 0);

y := actionFunc(x);

return table(y, { surname, forename });

end;



namesTable := dataset('x',namesRecord,FLAT);
output(processDs(namesTable));
output(processDs(namesTable, limitAndSortAction));

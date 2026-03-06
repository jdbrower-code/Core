' --------------
' IVR_inbound.vbs
' --------------
' This script will look in a folder for a CSV file and create a Call in Northstar if the 
' contents satisfies the format.
' CSV file must contain a first row of labels for the columns being imported.
' There MUST be a column called account and a column called occupant (not case sensitive).
' any other column label will be imported as schedule notes in the call.
' The note lines are 60 characters long. the combined length of the label and the content for
' any column must not exceed 58 characters (we are putting 2 chars between). 
' The note will be created like this... ( ": " is inserted between the Label and Content)
 
' LABEL: Content
' Example:
' File contains data that looks like this...

' Account,Occupant,My Label,Time,Action,Response
' 12345,0,IVR callout,20091225 13:01:25,Disconnect Warning,No Answer

' this file would add a Call to account 12345-0 with these lines in the schedule notes...

' My Label: IVR callout
' Time: 20091225 13:01:25
' Action: Disconnect Warning
' Response: No Answer

' after the file is processed successfully it will move it into a subfolder called "Archive".
' if the file does not process successfully it will move it to a subfolder called "Errors".

' A log file will be created each time this process is run that will describe the steps taken 
' and report any Errors.

' An IVR call will only be added once per account per day.
' If multiple records exist or are processed from multiple import files for an account 
' on the same day all notes will be appended to a single IVR call record for that day.

' resume on errors so they can be trapped in the log file. 
on error resume next
'-----------------------------------------------------------------------------------
' The sourceFolder is where the inbound IVR file is to be placed for processing.
sourceFolder = "C:\Inetpub\wwwroot\AlertWorks\LIVE"

' Enter the call code and call type to create in northstar to record the details from the inbound IVR file.
' Assumption: this is not creating an order, it will create a Logged call. 
' This callcode / calltype must be setup in NorthStar.
callCode = "CALLOG"
callType = ""

LogFile = "ivr_inbound.log"

' enter the ODBC Data Source
DSN="DSN=Northstar_live;Uid=harris_live;Pwd=unix88"

'-----------------------------------------------------------------------------------
set DBObj = CreateObject("ADODB.Connection")
DBObj.Open DSN
if Err.Number <> 0 then
      TSO.write now() & " -" & Err.Number & " " & Err.Description
      TSO.WriteLine ""
      Err.Clear
end if
Set FSO = CreateObject("Scripting.FileSystemObject")
Set TSO = FSO.OpenTextFile(LogFile, 8, True)

TSO.write now() & " -" & "-------------------------------------Started "
TSO.WriteLine ""

Set fso = CreateObject("Scripting.FileSystemObject")

if not fso.FolderExists(sourceFolder & "\Archive") then
   fso.CreateFolder(sourceFolder & "\Archive")
end if
if not fso.FolderExists(sourceFolder & "\Errors") then
   fso.CreateFolder(sourceFolder & "\Errors")
end if

Err.Clear
set folder = fso.GetFolder(sourceFolder)

  Set files = folder.Files
  csvcount = 0 
  For each CSV_file in files
   if lCase(fso.GetExtensionName(CSV_file.Name)) = "csv" then

      csvcount = csvcount + 1
      TSO.write now() & " -" & "Processing File...(" & CSV_file.Name & ")"
      TSO.WriteLine ""

      accountCol = -1
      
      'Open a file for reading 
      Set thefile = fso.OpenTextFile(CSV_file.Path,1 )
      labelstr = thefile.ReadLine

         callType = CSV_file.Name
         l_calltype = split(callType,"_")
         callType = l_calltype(0)
         callType = UCASE(callType)

      labelArr = Split(labelstr, ",")
      for x = 0 to UBound(labelArr)
           if Ucase(labelArr(x)) = "ACCOUNT_NO" then
	       accountCol = x
           end if
      next
      fileOK = true
      if accountCol = -1 then
	   fileOK = false
      end if
      if fileOK then
        updateCount = 0
        addCount = 0
	badCount = 0
	totCount = 0
        do while not thefile.AtEndOfStream
         totCount = totCount + 1
         ivrdata = thefile.ReadLine
         ivrdataArr = Split(ivrdata,",")

	 accountOccupant = ivrdataArr(accountCol)
	 accOccArr = Split(accountOccupant,"-")
	 accountNo = accOccArr(0)
	 occupantCode = accOccArr(1)

	 OKadd = true

         l_createyear = year(now()) 
         l_createmonth = month(now()) 
         l_createday = day(now()) 
         l_createhour = hour(now()) 
         l_createmin = minute(now()) 
		 l_createsecond = second(now())
         if l_createmonth < 10 then 
             l_createmonth = "0" & l_createmonth
         end if
         if l_createday < 10 then 
             l_createday = "0" & l_createday
         end if
         if l_createhour < 10 then 
             l_createhour = "0" & l_createhour
         end if
         if l_createmin < 10 then 
             l_createmin = "0" & l_createmin
         end if
		 if l_createsecond < 10 then
			 l_createsecond = "0" & l_createsecond
		 end if
         l_createhour = "00"
         l_createmin = "00"
		 l_createsecond = "00"
         l_datetime = l_createyear & "-" _ 
                    & l_createmonth & "-" _
                    & l_createday & " " _
                    & l_createhour & ":" _
                    & l_createmin & ":" _
					& l_createsecond
		l_datetime = FormatDateTime(l_datetime)
         l_name = "IVR"
         debtorNo = 0
	 l_sql = "select debtor_no, name " _
	       & " from pu_account " _
	       & " where account_no = " & accountNo _
	       & " and occupant_code = " & occupantCode 
         set getdetails = DBObj.execute(l_sql)
         if Err.Number <> 0 then
               TSO.write now() & " -" & Err.Number & " " & Err.Description
               TSO.WriteLine ""
               TSO.write l_sql
               TSO.WriteLine ""
               Err.Clear
         end if
         if not getdetails.eof then
             l_name = trim(replace(getdetails("name"), "'",""))
             debtorNo = getdetails("debtor_no")
         else
	    TSO.write now() & " -" & " Error: Account (" & accountNo & "-" & occupantCode & ") not found."
            TSO.WriteLine ""
	    badCount = badCount + 1
            OKadd = false
	 end if
         set getdetails = nothing
	 if OKadd then
             l_callno = 0
             ' check if a IVR Call already exists for this account today.
	     ' and update the notes on it instead of adding another IVR call.
	      l_sql = "select max(call_number) as call_number " _
	            & " from csrcalld " _
		    & " where account_no = " & accountNo _
		    & " and occupant_code = " & occupantCode _
		    & " and callcode = '" & callCode & "' " _
		    & " and calltype = '" & callType & "' " _
		    & " and createdon = '" & l_datetime & "' " 
              set callno = DBObj.execute(l_sql)
' wscript.echo l_sql
              if Err.Number <> 0 then
                    TSO.write now() & " -" & Err.Number & " " & Err.Description
                    TSO.WriteLine ""
                    TSO.write l_sql
                    TSO.WriteLine ""
                    Err.Clear
              end if
	      if not callno.eof then
		 if not isnull(callno("call_number")) then
                    l_callno=callno("call_number")
	         end if
	      end if
	      updateCount = updateCount + 1

              if l_callno = 0 then
	         updateCount = updateCount - 1
	         addCount = addCount + 1
                 l_sql = "insert into csrcalld (call_number, " _
                       &                      " calltype, " _
                       &                      " callcode, " _
                       &                      " note, " _
                       &                      " callername, " _
                       &                      " callstatus, " _
                       &                      " createdon, " _
                       &                      " createdby, " _
                       &                      " scheduledon, " _
                       &                      " requiredon, " _
                       &                      " account_no, " _
                       &                      " occupant_code, " _
                       &                      " debtor_no, " _
                       &                      " printed, " _
                       &                      " postbillto) " _
                       & " values (0, " _
                       &         " '" & callType & "'," _
                       &         " '" & callCode & "'," _
                       &         " 'IVR IMPORT'," _
                       &         " '" & l_name & "'," _
                       &         "'L' ," _
                       &         "'" & l_datetime & "'," _
                       &         " 'IVR'," _
                       &          "'" & l_datetime & "'," _
                       &         "'" & l_datetime & "'," _
                       &         accountNo & "," _
                       &         occupantCode & "," _
                       &         debtorNo & "," _
                       &         " 0,1)" 
                 set addcall = DBObj.execute(l_sql)
                 if Err.Number <> 0 then
                    TSO.write now() & " -" & Err.Number & " " & Err.Description
                    TSO.WriteLine ""
                    TSO.write l_sql
                    TSO.WriteLine ""
                    Err.Clear
                 end if
	         l_sql = "select max(call_number) as call_number " _
	               & " from csrcalld " _
		       & " where account_no = " & accountNo _
		       & " and occupant_code = " & occupantCode _
		       & " and callcode = '" & callCode & "' " _
		       & " and calltype = '" & callType & "' " _
		       & " and createdon = '" & l_datetime & "' " 
                 set callno = DBObj.execute(l_sql)
                 if Err.Number <> 0 then
                       TSO.write now() & " -" & Err.Number & " " & Err.Description
                       TSO.WriteLine ""
                       TSO.write l_sql
                       TSO.WriteLine ""
                       Err.Clear
                 end if
                 l_callno=callno("call_number")
             end if

         seq_no=1
	 l_sql = "select max(sequence_no) as seq " _
	       & " from csrschdd " _
	       & " where call_number = " & l_callno
         set getseq = DBObj.execute(l_sql)
         if Err.Number <> 0 then
            TSO.write now() & " -" & Err.Number & " " & Err.Description
            TSO.WriteLine ""
            TSO.write l_sql
            TSO.WriteLine ""
            Err.Clear
         end if
	 if not getseq.eof then
            if not isnull(getseq("seq")) then
                seq_no=getseq("seq") + 1
	    end if
	 end if
	 for x = 0 to UBound(ivrdataArr)
              if x <> accountCol and x <> occupantCol then

	          NoteTEXT = labelArr(x) & ": " & ivrdataArr(x)
				  NoteTEXT = replace(NoteTEXT,"Chr(34)","")
                  NoteTEXT = replace(NoteTEXT,"'","")
				  NoteTEXT = replace(NoteTEXT,",","")
                  l_sql = "insert into csrschdd (call_number, sequence_no, notes) " _
                            & " values ("  & l_callno & "," & seq_no & ",'" & NoteTEXT & "')"
                  set oRssql = DBObj.Execute(l_sql)
                  if Err.Number <> 0 then
                        TSO.write now() & " -" & Err.Number & " " & Err.Description
                        TSO.WriteLine ""
                        TSO.write l_sql
                        TSO.WriteLine ""
                        Err.Clear
                  end if
                  set oRssql = nothing
                  seq_no=seq_no+1

	      end if
	 next
        end if

        loop

	TSO.write now() & " -" & "Accounts (" & totCount & ")   Adds (" & addCount & ")   Updates (" & updateCount & ")   Bad(" & badCount & ")"
        TSO.WriteLine ""
        TSO.write now() & " -" & "Moving (" & CSV_file.Name & ") to Archive."
        TSO.WriteLine ""
  
        checkfile = CSV_file.Name
        archiveok = false
        x = 2
        do while not archiveok
           if fso.FileExists(sourceFolder & "\Archive\" & checkfile) then
               checkfile = fso.GetBaseName(CSV_file) & "_" & x & ".csv"
           else
               archiveok = true
           end if
           x = x + 1
        loop

	thefile.close
        CSV_file.Move sourceFolder & "\Archive\" & checkfile
	if Err.Number <> 0 then
	 TSO.write now() & " -" & "ERROR: Moving (" & CSV_file.Name & ") to Archive."
	 TSO.write " -" & Err.Number & " " & Err.Description
         TSO.WriteLine ""
         Err.Clear
	end if

      else
	thefile.close
	TSO.write now() & " -" & "ERROR: Processing (" & CSV_file.Name & ") -Bad data."
        TSO.WriteLine ""
	 
	checkfile = CSV_file.Name
        archiveok = false
        x = 2
        do while not archiveok
           if fso.FileExists(sourceFolder & "\Errors\" & checkfile) then
               checkfile = fso.GetBaseName(CSV_file) & "_" & x & ".csv"
           else
               archiveok = true
           end if
           x = x + 1
        loop

        TSO.write now() & " -" & "Moving (" & CSV_file.Name & ") to Errors."
        TSO.WriteLine ""

        CSV_file.Move sourceFolder & "\Errors\" & checkfile
	if Err.Number <> 0 then
	 TSO.write now() & " -" & "ERROR: Moving (" & CSV_file.Name & ") to Errors."
	 TSO.write " -" & Err.Number & " " & Err.Description
         TSO.WriteLine ""
         Err.Clear
        end if
      end if

   end if
  Next
  set folder = nothing

  if csvcount = 0 then
      TSO.write now() & " -" & "No files to process."
      TSO.WriteLine ""
  end if

  TSO.write now() & " -" & "-------------------------------------Finished "
  TSO.WriteLine ""
  TSO.WriteLine ""
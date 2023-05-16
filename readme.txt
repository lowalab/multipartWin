https://learn.microsoft.com/en-us/previous-versions/office/developer/exchange-server-2010/aa493937(v=exchg.140)

Content-Type: Multipart/related; boundary="boundary-content_example";
type=Text/HTML; start=example@somplace.com
;Content-Base header not allowed here
;since this is a multipart MIME object
--boundary-content_example  
Part 1:
Content-Type: Text/HTML; charset=US-ASCII
Content-ID: <example@somplace.com> 
Content-Location: http://www.webpage/images/the.one
; This Content-Location must contain an absolute URL, 
; since no base; is valid here.  
--boundary-content_example  
Part 2: 
Content-Type: Text/HTML; charset=US-ASCII 
Content-ID: <example2@somplace.com> 
Content-Location: the.one ; The Content-Base below applies to 
; this relative URL 
Content-Base: http://www.webpage/images/  
--boundary-content_example--  
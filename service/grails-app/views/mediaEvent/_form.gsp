<%@ page import="videochat.MediaEvent" %>



<div class="fieldcontain ${hasErrors(bean: mediaEventInstance, field: 'uid', 'error')} required">
	<label for="uid">
		<g:message code="mediaEvent.uid.label" default="Uid" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="uid" maxlength="50" required="" value="${mediaEventInstance?.uid}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaEventInstance, field: 'sessionId', 'error')} required">
	<label for="sessionId">
		<g:message code="mediaEvent.sessionId.label" default="Session Id" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="sessionId" maxlength="200" required="" value="${mediaEventInstance?.sessionId}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaEventInstance, field: 'mediaServiceId', 'error')} required">
	<label for="mediaServiceId">
		<g:message code="mediaEvent.mediaServiceId.label" default="Media Service Id" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="mediaServiceId" maxlength="200" required="" value="${mediaEventInstance?.mediaServiceId}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaEventInstance, field: 'dateLive', 'error')} ">
	<label for="dateLive">
		<g:message code="mediaEvent.dateLive.label" default="Date Live" />
		
	</label>
	<g:datePicker name="dateLive" precision="day"  value="${mediaEventInstance?.dateLive}" default="none" noSelection="['': '']" />
</div>

<div class="fieldcontain ${hasErrors(bean: mediaEventInstance, field: 'dateLeave', 'error')} ">
	<label for="dateLeave">
		<g:message code="mediaEvent.dateLeave.label" default="Date Leave" />
		
	</label>
	<g:datePicker name="dateLeave" precision="day"  value="${mediaEventInstance?.dateLeave}" default="none" noSelection="['': '']" />
</div>

<div class="fieldcontain ${hasErrors(bean: mediaEventInstance, field: 'duration', 'error')} required">
	<label for="duration">
		<g:message code="mediaEvent.duration.label" default="Duration" />
		<span class="required-indicator">*</span>
	</label>
	<g:field name="duration" type="number" min="0" value="${mediaEventInstance.duration}" required=""/>
</div>


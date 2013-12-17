<%@ page import="videochat.Online" %>



<div class="fieldcontain ${hasErrors(bean: onlineInstance, field: 'uid', 'error')} required">
	<label for="uid">
		<g:message code="online.uid.label" default="Uid" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="uid" maxlength="50" required="" value="${onlineInstance?.uid}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: onlineInstance, field: 'mediaServiceId', 'error')} required">
	<label for="mediaServiceId">
		<g:message code="online.mediaServiceId.label" default="Media Service Id" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="mediaServiceId" maxlength="200" required="" value="${onlineInstance?.mediaServiceId}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: onlineInstance, field: 'sessionId', 'error')} required">
	<label for="sessionId">
		<g:message code="online.sessionId.label" default="Session Id" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="sessionId" maxlength="200" required="" value="${onlineInstance?.sessionId}"/>
</div>


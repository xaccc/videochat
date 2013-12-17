<%@ page import="videochat.MediaBackup" %>



<div class="fieldcontain ${hasErrors(bean: mediaBackupInstance, field: 'uid', 'error')} required">
	<label for="uid">
		<g:message code="mediaBackup.uid.label" default="Uid" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="uid" maxlength="50" required="" value="${mediaBackupInstance?.uid}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaBackupInstance, field: 'domain', 'error')} required">
	<label for="domain">
		<g:message code="mediaBackup.domain.label" default="Domain" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="domain" maxlength="200" required="" value="${mediaBackupInstance?.domain}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaBackupInstance, field: 'sessionId', 'error')} required">
	<label for="sessionId">
		<g:message code="mediaBackup.sessionId.label" default="Session Id" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="sessionId" maxlength="200" required="" value="${mediaBackupInstance?.sessionId}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaBackupInstance, field: 'flvid', 'error')} required">
	<label for="flvid">
		<g:message code="mediaBackup.flvid.label" default="Flvid" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="flvid" maxlength="200" required="" value="${mediaBackupInstance?.flvid}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaBackupInstance, field: 'first', 'error')} required">
	<label for="first">
		<g:message code="mediaBackup.first.label" default="First" />
		<span class="required-indicator">*</span>
	</label>
	<g:checkBox name="first" value="${mediaBackupInstance?.first}" />
</div>

<div class="fieldcontain ${hasErrors(bean: mediaBackupInstance, field: 'backupDate', 'error')} required">
	<label for="backupDate">
		<g:message code="mediaBackup.backupDate.label" default="Backup Date" />
		<span class="required-indicator">*</span>
	</label>
	<g:datePicker name="backupDate" precision="day"  value="${mediaBackupInstance?.backupDate}"  />
</div>


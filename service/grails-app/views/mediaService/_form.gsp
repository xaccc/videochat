<%@ page import="videochat.MediaService" %>



<div class="fieldcontain ${hasErrors(bean: mediaServiceInstance, field: 'name', 'error')} required">
	<label for="name">
		<g:message code="mediaService.name.label" default="Name" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="name" maxlength="50" required="" value="${mediaServiceInstance?.name}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaServiceInstance, field: 'scheme', 'error')} required">
	<label for="scheme">
		<g:message code="mediaService.scheme.label" default="Scheme" />
		<span class="required-indicator">*</span>
	</label>
	<g:textField name="scheme" maxlength="50" required="" value="${mediaServiceInstance?.scheme}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaServiceInstance, field: 'domain', 'error')} required">
	<label for="domain">
		<g:message code="mediaService.domain.label" default="Domain" />
		<span class="required-indicator">*</span>
	</label>
	<g:textArea name="domain" cols="40" rows="5" maxlength="255" required="" value="${mediaServiceInstance?.domain}"/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaServiceInstance, field: 'port', 'error')} required">
	<label for="port">
		<g:message code="mediaService.port.label" default="Port" />
		<span class="required-indicator">*</span>
	</label>
	<g:field name="port" type="number" min="1" max="65535" format="0" value="${mediaServiceInstance.port}" required=""/>
</div>

<div class="fieldcontain ${hasErrors(bean: mediaServiceInstance, field: 'path', 'error')} required">
	<label for="path">
		<g:message code="mediaService.path.label" default="Path" />
		<span class="required-indicator">*</span>
	</label>
	<g:textArea name="path" cols="40" rows="5" maxlength="255" required="" value="${mediaServiceInstance?.path}"/>
</div>


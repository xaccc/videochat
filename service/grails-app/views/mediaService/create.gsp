<%@ page import="videochat.MediaService" %>
<!DOCTYPE html>
<html>
	<head>
		<meta name="layout" content="main">
		<g:set var="entityName" value="${message(code: 'mediaService.label', default: 'MediaService')}" />
		<title><g:message code="default.create.label" args="[entityName]" /></title>
	</head>
	<body>
		<a href="#create-mediaService" class="skip" tabindex="-1"><g:message code="default.link.skip.label" default="Skip to content&hellip;"/></a>
		<div class="nav" role="navigation">
			<ul>
				<li><g:link class="list" action="list" controller="mediaService"><g:message code="default.list.label" args="[message(code: 'mediaService.label', default: 'Services')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="mediaBackup"><g:message code="default.list.label" args="[message(code: 'mediaBackup.label', default: 'Backups')]" /></g:link></li>
				<li><g:link class="list" action="list" controller="online"><g:message code="default.list.label" args="[message(code: 'online.label', default: 'Onlines')]" /></g:link></li>
			</ul>
		</div>
		<div id="create-mediaService" class="content scaffold-create" role="main">
			<h1><g:message code="default.create.label" args="[entityName]" /></h1>
			<g:if test="${flash.message}">
			<div class="message" role="status">${flash.message}</div>
			</g:if>
			<g:hasErrors bean="${mediaServiceInstance}">
			<ul class="errors" role="alert">
				<g:eachError bean="${mediaServiceInstance}" var="error">
				<li <g:if test="${error in org.springframework.validation.FieldError}">data-field-id="${error.field}"</g:if>><g:message error="${error}"/></li>
				</g:eachError>
			</ul>
			</g:hasErrors>
			<g:form action="save" >
				<fieldset class="form">
					<g:render template="form"/>
				</fieldset>
				<fieldset class="buttons">
					<g:submitButton name="create" class="save" value="${message(code: 'default.button.create.label', default: 'Create')}" />
				</fieldset>
			</g:form>
		</div>
	</body>
</html>

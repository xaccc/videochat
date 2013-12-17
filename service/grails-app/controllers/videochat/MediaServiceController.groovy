package videochat

import org.springframework.dao.DataIntegrityViolationException

class MediaServiceController {

    static allowedMethods = [save: "POST", update: "POST", delete: "POST"]

    def index() {
        redirect(action: "list", params: params)
    }

    def list(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        [mediaServiceInstanceList: MediaService.list(params), mediaServiceInstanceTotal: MediaService.count()]
    }

    def create() {
        [mediaServiceInstance: new MediaService(params)]
    }

    def save() {
        def mediaServiceInstance = new MediaService(params)
        if (!mediaServiceInstance.save(flush: true)) {
            render(view: "create", model: [mediaServiceInstance: mediaServiceInstance])
            return
        }

        flash.message = message(code: 'default.created.message', args: [message(code: 'mediaService.label', default: 'MediaService'), mediaServiceInstance.id])
        redirect(action: "show", id: mediaServiceInstance.id)
    }

    def show(String id) {
        def mediaServiceInstance = MediaService.get(id)
        if (!mediaServiceInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaService.label', default: 'MediaService'), id])
            redirect(action: "list")
            return
        }

        [mediaServiceInstance: mediaServiceInstance]
    }

    def edit(String id) {
        def mediaServiceInstance = MediaService.get(id)
        if (!mediaServiceInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaService.label', default: 'MediaService'), id])
            redirect(action: "list")
            return
        }

        [mediaServiceInstance: mediaServiceInstance]
    }

    def update(String id, Long version) {
        def mediaServiceInstance = MediaService.get(id)
        if (!mediaServiceInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaService.label', default: 'MediaService'), id])
            redirect(action: "list")
            return
        }

        if (version != null) {
            if (mediaServiceInstance.version > version) {
                mediaServiceInstance.errors.rejectValue("version", "default.optimistic.locking.failure",
                          [message(code: 'mediaService.label', default: 'MediaService')] as Object[],
                          "Another user has updated this MediaService while you were editing")
                render(view: "edit", model: [mediaServiceInstance: mediaServiceInstance])
                return
            }
        }

        mediaServiceInstance.properties = params

        if (!mediaServiceInstance.save(flush: true)) {
            render(view: "edit", model: [mediaServiceInstance: mediaServiceInstance])
            return
        }

        flash.message = message(code: 'default.updated.message', args: [message(code: 'mediaService.label', default: 'MediaService'), mediaServiceInstance.id])
        redirect(action: "show", id: mediaServiceInstance.id)
    }

    def delete(String id) {
        def mediaServiceInstance = MediaService.get(id)
        if (!mediaServiceInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaService.label', default: 'MediaService'), id])
            redirect(action: "list")
            return
        }

        try {
            mediaServiceInstance.delete(flush: true)
            flash.message = message(code: 'default.deleted.message', args: [message(code: 'mediaService.label', default: 'MediaService'), id])
            redirect(action: "list")
        }
        catch (DataIntegrityViolationException e) {
            flash.message = message(code: 'default.not.deleted.message', args: [message(code: 'mediaService.label', default: 'MediaService'), id])
            redirect(action: "show", id: id)
        }
    }
}

package videochat

import org.springframework.dao.DataIntegrityViolationException

class MediaBackupController {

    static allowedMethods = [save: "POST", update: "POST", delete: "POST"]

    def index() {
        redirect(action: "list", params: params)
    }

    def list(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        [mediaBackupInstanceList: MediaBackup.list(params), mediaBackupInstanceTotal: MediaBackup.count()]
    }
    
    def listBySession(String id,Integer max) {
        params.max = Math.min(max ?: 10, 100)
        render(view:'list', model:[mediaBackupInstanceList: MediaBackup.findAllBySessionId(id, params), mediaBackupInstanceTotal: MediaBackup.countBySessionId(id)])
    }
    
    def listByUid(String id,Integer max) {
        params.max = Math.min(max ?: 10, 100)
        render(view:'list', model:[mediaBackupInstanceList: MediaBackup.findAllByUid(id, params), mediaBackupInstanceTotal: MediaBackup.countByUid(id)])
    }    

    def create() {
        [mediaBackupInstance: new MediaBackup(params)]
    }

    def save() {
        def mediaBackupInstance = new MediaBackup(params)
        if (!mediaBackupInstance.save(flush: true)) {
            render(view: "create", model: [mediaBackupInstance: mediaBackupInstance])
            return
        }

        flash.message = message(code: 'default.created.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), mediaBackupInstance.id])
        redirect(action: "show", id: mediaBackupInstance.id)
    }

    def show(String id) {
        def mediaBackupInstance = MediaBackup.get(id)
        if (!mediaBackupInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), id])
            redirect(action: "list")
            return
        }

        [mediaBackupInstance: mediaBackupInstance]
    }

    def edit(String id) {
        def mediaBackupInstance = MediaBackup.get(id)
        if (!mediaBackupInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), id])
            redirect(action: "list")
            return
        }

        [mediaBackupInstance: mediaBackupInstance]
    }

    def update(String id, Long version) {
        def mediaBackupInstance = MediaBackup.get(id)
        if (!mediaBackupInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), id])
            redirect(action: "list")
            return
        }

        if (version != null) {
            if (mediaBackupInstance.version > version) {
                mediaBackupInstance.errors.rejectValue("version", "default.optimistic.locking.failure",
                          [message(code: 'mediaBackup.label', default: 'MediaBackup')] as Object[],
                          "Another user has updated this MediaBackup while you were editing")
                render(view: "edit", model: [mediaBackupInstance: mediaBackupInstance])
                return
            }
        }

        mediaBackupInstance.properties = params

        if (!mediaBackupInstance.save(flush: true)) {
            render(view: "edit", model: [mediaBackupInstance: mediaBackupInstance])
            return
        }

        flash.message = message(code: 'default.updated.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), mediaBackupInstance.id])
        redirect(action: "show", id: mediaBackupInstance.id)
    }

    def delete(String id) {
        def mediaBackupInstance = MediaBackup.get(id)
        if (!mediaBackupInstance) {
            flash.message = message(code: 'default.not.found.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), id])
            redirect(action: "list")
            return
        }

        try {
            mediaBackupInstance.delete(flush: true)
            flash.message = message(code: 'default.deleted.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), id])
            redirect(action: "list")
        }
        catch (DataIntegrityViolationException e) {
            flash.message = message(code: 'default.not.deleted.message', args: [message(code: 'mediaBackup.label', default: 'MediaBackup'), id])
            redirect(action: "show", id: id)
        }
    }
}

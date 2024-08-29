function init(): void {
  window.addEventListener('DOMContentLoaded', () => {
    doAThing()
  })
}

function doAThing(): void {
  window.document.body.addEventListener('contextmenu', () => {
    window.api.openContextMenu();
  })
}

init()

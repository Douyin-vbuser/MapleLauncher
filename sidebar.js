const SIDEBAR_ITEMS = [
    { label: '启动', action: 'start' },
    { label: '下载', action: 'download' },
    { label: '登录', action: 'login' },
    { label: '设置', action: 'settings' }
];

const SIDEBAR_ITEM_CLASSES = 'p-4 hover:bg-green-300';
const SIDEBAR_WIDTH = 'w-1/4';
const MAIN_CONTENT_WIDTH = 'w-3/4';
const SIDEBAR_BG_COLOR = 'bg-black';
const MAIN_CONTENT_BG_COLOR = 'bg-zinc-800';
const TEXT_COLOR = 'text-white';
const FULL_HEIGHT = 'h-screen';
const FLEX_CONTAINER = 'flex';

class CustomSidebar extends HTMLElement {
    connectedCallback() {
        this.render();
    }

    render() {
        this.innerHTML = `
            <div class="${FLEX_CONTAINER}">
                <div class="${SIDEBAR_BG_COLOR} ${TEXT_COLOR} ${SIDEBAR_WIDTH} ${FULL_HEIGHT}">
                    <ul>
                        ${SIDEBAR_ITEMS.map(item => `<li class="${SIDEBAR_ITEM_CLASSES}" data-action="${item.action}">${item.label}</li>`).join('')}
                    </ul>
                </div>
                <div class="${MAIN_CONTENT_BG_COLOR} ${TEXT_COLOR} ${MAIN_CONTENT_WIDTH} ${FULL_HEIGHT}">
                    
                </div>
            </div>
        `;
    }
}

customElements.define('custom-sidebar', CustomSidebar);
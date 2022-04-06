export function getAppInstallerUri(): string
export function getWindowsVersion(): { major: number; minor: number; build: number }
export function setWindowBlur(hwnd: ArrayBuffer, effect: AccentState): boolean
export function setMica(hwnd: ArrayBuffer, enabled: boolean): boolean
export function createShortcut(exePath: string, destination: string, arguments: string, description: string, cwd: string, iconPath: string): boolean

declare enum AccentState
{
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6
}
